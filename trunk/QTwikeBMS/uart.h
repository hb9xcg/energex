/***************************************************************************
 *   Copyright (C) 2008 by Markus Walser                                   *
 *   markus.walser@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef UART_H_
#define UART_H_

#include "types.h"

#define BAUDRATE	9600	/*!< Baudrate fuer UART-Kommunikation */
	
/*!
 * @brief		Sende Kommando per UART im Little Endian
 * @param cmd	Zeiger auf das Kommando
 * @return 	Anzahl der gesendete Bytes
 */
//int uart_send_cmd(command_t *cmd);
//#define uart_send_cmd(cmd)  uart_write((uint8*)cmd,sizeof(command_t));

/*!
 * @brief			Sendet Daten per UART im Little Endian
 * @param data		Datenpuffer
 * @param length	Groesse des Datenpuffers in Bytes
 */
void uart_write(const uint8_t* data, uint8_t length);

/*!
 * @brief			Liest Zeichen von der UART
 * @param data		Der Zeiger an den die gelesenen Zeichen kommen
 * @param length	Anzahl der zu lesenden Bytes
 * @return			Anzahl der tatsaechlich gelesenen Zeichen
 */
int16 uart_read(uint8* data, int16 length);

/*!
 * @brief	Initialisiert den UART und aktiviert Receiver und Transmitter sowie den Receive-Interrupt. 
 * Die Ein- und Ausgebe-FIFO werden initialisiert. Das globale Interrupt-Enable-Flag (I-Bit in SREG) wird nicht veraendert.
 */
void uart_init(void);

/*!
 * @brief	Wartet, bis die Uebertragung fertig ist.
 */
void uart_flush(void);

/*! 
 * @brief	Prueft, ob Daten verfuegbar 
 * @return	Anzahl der verfuegbaren Bytes
 */
uint8 uart_data_available(void);

#endif /*UART_H_*/
