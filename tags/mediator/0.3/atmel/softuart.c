/*
 * Energex
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
 * @file 	softuart.c 
 * @brief 	Implementierung einer Software-UART
 * @author 	http://www.roboternetz.de/wissen/index.php/Software-UART_mit_avr-gcc
 * @date 	04.10.2008
 */
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef SIGNAL
#include <avr/signal.h>
#endif // SIGNAL 

#include "softuart.h"

#include "fifo.h" 

#define BAUDRATE 2400

#define nop() __asm volatile ("nop")

#define SUART_TXD_PORT PORTD
#define SUART_TXD_DDR  DDRD
#define SUART_TXD_BIT  PD5
static volatile uint16_t outframe;

#define SUART_RXD_PORT PORTD
#define SUART_RXD_PIN  PIND
#define SUART_RXD_DDR  DDRD
#define SUART_RXD_BIT  PB6
static volatile uint16_t inframe;
static volatile uint8_t inbits, received;


#define INBUF_SIZE 8
static uint8_t inbuf[INBUF_SIZE];
fifo_t infifo;



// Initialisierung für einen ATmega644 
// Für andere AVR-Derivate sieht dies vermutlich anders aus: 
// Registernamen ändern sich (zB TIMSK anstatt TIMSK1, etc). 
void softuart_init(void)
{
    fifo_init(&infifo, inbuf, INBUF_SIZE);

    uint8_t tifr = 0;
    uint8_t sreg = SREG;
    cli();

    // Mode #4 für Timer1 
    // und volle MCU clock 
    // IC Noise Cancel 
    // IC on Falling Edge 
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10) | (0 << ICES1) | (1 << ICNC1);

    // OutputCompare für gewünschte Timer1 Frequenz 
    OCR1A = (uint16_t) ((uint32_t) F_CPU/BAUDRATE);

    // Setup RX
    SUART_RXD_DDR  &= ~(1 << SUART_RXD_BIT);
    SUART_RXD_PORT |=  (1 << SUART_RXD_BIT);
    TIMSK1 |= (1 << ICIE1); // MAW: TICIE1
    tifr  |= (1 << ICF1) | (1 << OCF1B);

    // Setup TX
    tifr |= (1 << OCF1A);
    SUART_TXD_PORT |= (1 << SUART_TXD_BIT);
    SUART_TXD_DDR  |= (1 << SUART_TXD_BIT);
    outframe = 0;

    TIFR1 = tifr;

    SREG = sreg;
}

// Transmitter
void softuart_putc (const char c)
{
    do
    {
        sei(); nop(); cli(); // yield(); 
    } while (outframe);

    // frame = *.P.7.6.5.4.3.2.1.0.S   S=Start(0), P=Stop(1), *=Endemarke(1) 
    outframe = (3 << 9) | (((uint8_t) c) << 1);

    TIMSK1 |= (1 << OCIE1A);
    TIFR1   = (1 << OCF1A);

    sei();
}

SIGNAL (SIG_OUTPUT_COMPARE1A)
{
    uint16_t data = outframe;
   
    if (data & 1)      SUART_TXD_PORT |=  (1 << SUART_TXD_BIT);
    else               SUART_TXD_PORT &= ~(1 << SUART_TXD_BIT);
   
    if (1 == data)
    {
        TIMSK1 &= ~(1 << OCIE1A);
    }   
   
    outframe = data >> 1;
}


// Receiver
SIGNAL (SIG_INPUT_CAPTURE1)
{
    uint16_t icr1  = ICR1;
    uint16_t ocr1a = OCR1A;
   
    // Eine halbe Bitzeit zu ICR1 addieren (modulo OCR1A) und nach OCR1B
    uint16_t ocr1b = icr1 + ocr1a/2;
    if (ocr1b >= ocr1a)
        ocr1b -= ocr1a;
    OCR1B = ocr1b;
   
    TIFR1 = (1 << OCF1B);
    TIMSK1 = (TIMSK1 & ~(1 << ICIE1)) | (1 << OCIE1B);
    inframe = 0;
    inbits = 0;
}

SIGNAL (SIG_OUTPUT_COMPARE1B)
{
    uint16_t data = inframe >> 1;
   
    if (SUART_RXD_PIN & (1 << SUART_RXD_BIT))
        data |= (1 << 9);
      
    uint8_t bits = inbits+1;
   
    if (10 == bits)
    {
        if ((data & 1) == 0)
            if (data >= (1 << 9))
            {
#ifdef _FIFO_H_         
                _inline_fifo_put (&infifo, data >> 1);
#else            
                indata = data >> 1;
#endif // _FIFO_H_            
                received = 1;
            }
      
        TIMSK1 = (TIMSK1 & ~(1 << OCIE1B)) | (1 << ICIE1);
        TIFR1 = (1 << ICF1);
    }
    else
    {
        inbits = bits;
        inframe = data;
    }
}


// FIFO
#ifdef SUART_RXD
#ifdef _FIFO_H_

int softuart_getc_wait()
{
    return (int) fifo_get_wait (&infifo);
}

int softuart_getc_nowait()
{
    return fifo_get_nowait (&infifo);
}

#else // _FIFO_H_

int softuart_getc_wait()
{
    while (!received)   {}
    received = 0;
   
    return (int) indata;
}

int softuart_getc_nowait()
{
    if (received)
    {
        received = 0;
        return (int) indata;
    }
   
    return -1;
}

#endif // _FIFO_H_
#endif // SUART_RXD


