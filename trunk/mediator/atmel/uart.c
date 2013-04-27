/*
 * Energex
 * 
 * Copyright (C) 2008-2011 by Markus Walser
 * markus.walser@gmail.com
 *
 * Copyright (C) 2005-2007 by Benjamin Benz
 * bbe@heise.de
 *
 * Copyright (C) 2005-2007 by Timo Sandmann
 * mail@timosandmann.de
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your
 * option) any later version. 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 * 
 */

/*! 
 * @file 	uart.c 
 * @brief 	Routinen zur seriellen Kommunikation
 * @author 	Benjamin Benz (bbe@heise.de)
 * @author	Timo Sandmann (mail@timosandmann.de)
 * @date 	26.12.2005
 */


#include "mediator.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef NEW_AVR_LIB
	#include <avr/signal.h>
#endif

#include "uart.h"
#include "command.h"
#include "os_thread.h"
//#include "log.h"
//#include "bot-2-pc.h"


#define BUFSIZE_IN 0x30
uint8_t inbuf[BUFSIZE_IN];	/*!< Eingangspuffer */
fifo_t infifo;				/*!< Eingangs-FIFO */
uint8_t simplex = 0;

#if BAUDRATE == 115200
#define BUFSIZE_OUT	0x70	// wer schneller sendet, braucht auch weniger Pufferspeicher ;)
#else
#define BUFSIZE_OUT 0x80
#endif	// BAUDRATE
uint8 outbuf[BUFSIZE_OUT];	/*!< Ausgangspuffer */
fifo_t outfifo;				/*!< Ausgangs-FIFO */

/*!
 * @brief	Initialisiert den UART und aktiviert Receiver und Transmitter sowie den Receive-Interrupt. 
 * Die Ein- und Ausgebe-FIFO werden initialisiert. Das globale Interrupt-Enable-Flag (I-Bit in SREG) wird nicht veraendert.
 */
void uart_init(void){	 
    /* FIFOs für Ein- und Ausgabe initialisieren */ 
    fifo_init(&infifo, inbuf, BUFSIZE_IN);
    fifo_init(&outfifo, outbuf, BUFSIZE_OUT);

    uint8 sreg = SREG;
    /* Interrupts kurz deaktivieren */ 
    cli();

    DDRB  |=  TRANSMIT_ENABLE; // Configure RS485 transmit/receive enable output
    PORTB &= ~TRANSMIT_ENABLE; // RS485-Transmitter ausschalten und RS485-Receiver einschalten

    UBRRH = (UART_CALC_BAUDRATE(BAUDRATE)>>8) & 0xFF;
    UBRRL = (UART_CALC_BAUDRATE(BAUDRATE) & 0xFF);
    
    /* UART Receiver und Transmitter anschalten, Receive-Interrupt aktivieren */ 
    UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
    /* Data mode 8N1, asynchron */
    uint8 ucsrc = (1 << UCSZ1) | (1 << UCSZ0);
    #ifdef URSEL
	ucsrc |= (1 << URSEL);	// fuer ATMega32
    #endif    
    UCSRC = ucsrc;

    /* Flush Receive-Buffer (entfernen evtl. vorhandener ungueltiger Werte) */ 
    do{
		UDR;	// UDR auslesen (Wert wird nicht verwendet)
    } while (UCSRA & (1 << RXC));

    /* Ruecksetzen von Receive und Transmit Complete-Flags */ 
    UCSRA = (1 << RXC) | (1 << TXC)
#ifdef UART_DOUBLESPEED
    		| (1<<U2X)
#endif
    		;
	
    /* Global Interrupt-Flag wiederherstellen */
    SREG = sreg;

}

/*!
 * @brief	Interrupthandler fuer eingehende Daten
 * Empfangene Zeichen werden in die Eingabgs-FIFO gespeichert und warten dort.
 */ 
#ifdef __AVR_ATmega644P__
	SIGNAL (USART0_RX_vect){		
#else
	SIGNAL (SIG_UART_RECV){
#endif
//	UCSRB &= ~(1 << RXCIE);	// diesen Interrupt aus (denn ISR ist nicht reentrant)
//	sei();					// andere Interrupts wieder an
//	if (infifo.count == BUFSIZE_IN){   
//		/* ERROR! Receive buffer full!
//		 * => Pufferinhalt erst verarbeiten - das funktioniert besser als es aussieht. ;-)
//		 * Ist allerdings nur dann clever, wenn das ausgewertete Command nicht mehr Daten per
//		 * uart_read() lesen will, als bereits im Puffer sind, denn der Interrupt ist ja aus... */
//		#ifdef BOT_2_PC_AVAILABLE
//			bot_2_pc_listen();		// Daten des Puffers auswerten
//		#endif
//	}
	_inline_fifo_put(&infifo, UDR);
//	UCSRB |= (1 << RXCIE);	// diesen Interrupt wieder an 	
}

/*!
 * @brief	Interrupthandler fuer ausgehende Daten
 * Ein Zeichen aus der Ausgabe-FIFO lesen und ausgeben.
 * Ist das Zeichen fertig ausgegeben, wird ein neuer SIG_UART_DATA-IRQ getriggert.
 * Ist die FIFO leer, deaktiviert die ISR ihren eigenen IRQ.
 */ 
#ifdef __AVR_ATmega644P__
	SIGNAL (USART0_UDRE_vect){
#else
	SIGNAL (SIG_UART_DATA){
#endif
	UCSRB &= ~(1 << UDRIE);	// diesen Interrupt aus (denn ISR ist nicht reentrant)
	sei();					// andere Interrupts wieder an
	if (outfifo.count > 0){
		UDR = _inline_fifo_get(&outfifo);
		UCSRB |= (1 << UDRIE);	// diesen Interrupt wieder an 
	}
}

/*!
 * @brief	Interrupthandler fuer wenn ausgehende Daten fertig gesendet wurden.
 * Schaltet den Empfangpfad wieder ein für den simplex RS485 Betrieb.
 */
SIGNAL (USART0_TX_vect)
{
	if (outfifo.count == 0){
		// RS485-Transmitter ausschalten und RS485-Receiver einschalten
		PORTB &= ~TRANSMIT_ENABLE;
		UCSRB |= (1 << RXEN);
	}

	// andere Interrupts wieder an
	sei();
}

/*!
 * @brief			Sendet Daten per UART im Little Endian
 * @param data		Datenpuffer
 * @param length	Groesse des Datenpuffers in Bytes
 */
void uart_write(const uint8* data, uint8 length){
	if (length > BUFSIZE_OUT){
		/* das ist zu viel auf einmal => teile und herrsche */
		uart_write(data, length/2);
		uart_write(data + length/2, length - length/2);
		return;
	} 
	/* falls Sendepuffer voll, diesen erst flushen */ 
	uint8 space = BUFSIZE_OUT - outfifo.count;
	if (space < length) uart_flush();
	/* Daten in Ausgangs-FIFO kopieren */
	fifo_put_data(&outfifo, data, length);

	// RS485-Transmitter einschalten und RS485-Receiver ausschalten
	UCSRB &= ~(1 << RXEN);
	PORTB |= TRANSMIT_ENABLE;
	
	/* Data empty und TX Complete Interrupt an */
	UCSRB |= (1 << UDRIE) | (1 << TXCIE0);	
}

/*!
 * @brief		Sendet Daten per UART mit Twike Timing.
 * @param data		Datenpuffer
 * @param length	Groesse des Datenpuffers in Bytes
 */
void uart_send_twike(const uint8_t* data, uint8_t length)
{
	os_thread_sleep(9); // Gets rounded to 10ms
	// RS485-Transmitter einschalten und RS485-Receiver ausschalten
	UCSRB &= ~((1 << RXEN) | (1 << TXCIE0));
	PORTB |= TRANSMIT_ENABLE;
	

	os_thread_sleep(5); // Gets rounded to 6ms

	uint8_t idx;

	for (idx=0; idx<length; idx++)
	{
		UDR = data[idx];
		os_thread_sleep(7); // Gets rounded to 8ms
	}

	// RS485-Transmitter ausschalten und RS485-Receiver einschalten
	PORTB &= ~TRANSMIT_ENABLE;
	UCSRB |= (1 << RXEN);
}
	
/*!
 * @brief	Wartet, bis die Uebertragung fertig ist.
 */
void uart_flush(void)
{
	while (UCSRB & (1 << UDRIE))
	{
		// Sending one byte with 1200bd takes about 8ms.
		os_thread_sleep(20);
	}
}

