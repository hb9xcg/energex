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
#include "uart.h"
#include <SerialPort.h>


static SerialPort serialPort( "/dev/ttyUSB0" );

/*!
 * @brief			Sendet Daten per UART im Little Endian
 * @param data		Datenpuffer
 * @param length	Groesse des Datenpuffers in Bytes
 */
void uart_write(const uint8_t* data, uint8_t length)
{
	for(int idx=0; idx<length; idx++)
	{
		serialPort.WriteByte( data[idx] );	
	}	
}

/*!
 * @brief			Liest Zeichen von der UART
 * @param data		Der Zeiger an den die gelesenen Zeichen kommen
 * @param length	Anzahl der zu lesenden Bytes
 * @return			Anzahl der tatsaechlich gelesenen Zeichen
 */
int16 uart_read(uint8* data, int16 length)
{
	for(int idx=0; idx<length; idx++)
	{
		const unsigned int timeout = 0;
		data[idx] = serialPort.ReadByte( timeout );
	}
	
	return length;
}

/*!
 * @brief	Initialisiert den UART und aktiviert Receiver und Transmitter sowie den Receive-Interrupt. 
 * Die Ein- und Ausgebe-FIFO werden initialisiert. Das globale Interrupt-Enable-Flag (I-Bit in SREG) wird nicht veraendert.
 */
void uart_init(void)
{
	serialPort.Open();
	serialPort.SetBaudRate( SerialPort::BAUD_2400 );
}

/*!
 * @brief	Wartet, bis die Uebertragung fertig ist.
 */
void uart_flush(void)
{
	
}

/*! 
 * @brief	Prueft, ob Daten verfuegbar 
 * @return	Anzahl der verfuegbaren Bytes
 */
uint8 uart_data_available(void)
{
	return serialPort.IsDataAvailable();
}
