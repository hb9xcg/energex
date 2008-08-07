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
#include "ReceiverThread.h"
#include "uart.h"
//#include "os_thread.h"
#include "protocol.h"
#include "battery.h"


uint8_t address;
uint8_t command;
uint8_t parameter;
uint8_t length;
uint8_t data[8];
uint8_t checksum;

enum EPacketState
{
	eExpectStart,
	eExpectAddress,
	eExpectCommand,
	eExpectData,
	eExpectChecksum
};

ReceiverThread::ReceiverThread()
{
	uart_init();
	
	setParameterValue(&battery1, BUS_ADRESSE, 0x31);
	setParameterValue(&battery2, BUS_ADRESSE, 0x32);
	setParameterValue(&battery3, BUS_ADRESSE, 0x34);
}

ReceiverThread::~ReceiverThread()
{
	running = false;
	while(isRunning());
}

void ReceiverThread::stop(void)
{
	running = false;
}

void ReceiverThread::os_thread_sleep(uint32_t sleep)
{
	msleep(sleep);
}



/************************ Simulation Code *****************************/
void ReceiverThread::run ()
{
	EPacketState eState = eExpectStart;
	uint8_t character, idx;
	
	running = true;
	while(running)
	{
		if(uart_data_available() > 0)
		{
			uart_read(&character, 1);

			switch(eState)
			{
			case eExpectStart:
				switch(character)
				{
				case FRAME:
					eState = eExpectAddress;
					break;
				}
				break;
				
			case eExpectAddress:
				switch(character)
				{
				case BATTERY_1:
				case BATTERY_2:
				case BATTERY_3:
				case BROADCAST:
					address = character;
					eState  = eExpectCommand;
					break;
				case FRAME:
					eState = eExpectAddress;
					break;
				default:
					eState = eExpectStart;
				}
				break;
				
			case eExpectCommand:
				switch(character)
				{
				case REQ_DATA:
				case REQ_GROUP:
				case TRM_DATA:
				case TRM_GROUP:
					command  = character;
					checksum = character;
					length   = character & LENGTH_MASK;
					idx      = 0;
					eState   = eExpectData;
					break;
				case FRAME:
					eState = eExpectAddress;
					break;
				default:
					eState = eExpectStart;
				}
				break;
				
			case eExpectData:
				checksum ^= character;
				if(  idx == 0 || 
					  data[idx-1] != FRAME || 
					  character != FRAME )
				{
					data[idx++] = character;
				}
				if(idx >= length)
				{
					eState = eExpectChecksum;
				}
				break;
			
			case eExpectChecksum:
				if(checksum==character)
				{
					receivePacket();
				}
				eState = eExpectStart;
				break;
			}
		}
		
		os_thread_sleep(10);
	}
}

void ReceiverThread::receivePacket()
{
	logReceive();
	
	parameter = data[1];
	
	if(address==BROADCAST)
	{
		switch(command)
		{
		case TRM_DATA:
			receiveData();
			break;
		default:
			break;
		}
	}
	else
	{
		switch(command)
		{
		case REQ_DATA:
			transmitData();
			break;
		case REQ_GROUP:
			transmitGroup();
			break;
		case TRM_DATA:
		case TRM_GROUP:
			break;
		}	
	}	
}

void ReceiverThread::receiveData()
{
	uint16_t value;
	
	value = data[3] | (data[2]<<8);
	
	if(address==BROADCAST)
	{
		setParameterValue(&battery1, parameter, value);
		setParameterValue(&battery2, parameter, value);
		setParameterValue(&battery3, parameter, value);
	}
	else
	{
		switch(address)
		{
		case BATTERY_1:	setParameterValue(&battery1, parameter, value);
			break;
		case BATTERY_2:	setParameterValue(&battery2, parameter, value);
			break;
		case BATTERY_3:	setParameterValue(&battery3, parameter, value); 
			break;
		}
	}
}

void ReceiverThread::transmitData()
{
	char packet[16];
	battery_t* pBattery;
	int16_t value;
	int8_t checksum=0, i;
	
	switch(address)
	{
	case BATTERY_1:	pBattery = &battery1; break;
	case BATTERY_2:	pBattery = &battery2; break;
	case BATTERY_3:	pBattery = &battery3; break;
	}
	
	value = getParameterValue(parameter, pBattery);
	
	packet[ 0] = FRAME;
	packet[ 1] = address;
	packet[ 2] = TRM_DATA;
	packet[ 3] = 0;
	packet[ 4] = parameter;
	packet[ 5] = value >> 8;
	packet[ 6] = value & 0xff;
	
	for(i=2; i<=6; i++)
		checksum ^= packet[i];
	
	packet[7] = checksum;
	
	uart_write( (uint8_t*)packet, 8);
	
	logTransmit((uint8_t*)packet, 8);
}

void ReceiverThread::transmitGroup()
{
	char packet[16];
	battery_t* pBattery;
	int16_t value;
	int8_t checksum=0, i;
	
	switch(address)
	{
	case BATTERY_1:	pBattery = &battery1; break;
	case BATTERY_2:	pBattery = &battery2; break;
	case BATTERY_3:	pBattery = &battery3; break;
	}
	
	value = getParameterValue(parameter, pBattery);
	
	packet[ 0] = FRAME;
	packet[ 1] = address;
	packet[ 2] = TRM_DATA;
	packet[ 3] = 0;
	packet[ 4] = 0;
	packet[ 5] = pBattery->current >> 8;
	packet[ 6] = pBattery->current & 0xff;
	packet[ 7] = pBattery->voltage >> 8;
	packet[ 8] = pBattery->voltage & 0xff;
	packet[ 9] = value >> 8;
	packet[10] = value & 0xff;
	
	for(i=2; i<=10; i++)
		checksum ^= packet[i];
	
	packet[10] = checksum;
	
	uart_write( (uint8_t*)packet, 11);
	
	logTransmit((uint8_t*)packet, 11);
}

void ReceiverThread::logTransmit(uint8_t packet[], uint8_t length)
{
	updateLog("Transmitting: ");
	for(int i=0; i<length; i++) {
		uint8_t character = packet[i];
		updateLog(QString("%1 ").arg(character, 0, 16));
	}
	updateLog(QString("%1").arg('\n'));
}

void ReceiverThread::logReceive()
{
	uint8_t idx;
	updateLog("Received: ");
	updateLog(QString("%1 ").arg(0x10, 0, 16));
	updateLog(QString("%1 ").arg(address, 0, 16));
	updateLog(QString("%1 ").arg(command, 0, 16));
	for(idx=0; idx<length; idx++) {
		uint8_t character = data[idx];
		updateLog(QString("%1 ").arg(character, 0, 16));
	}
	updateLog(QString("%1 ").arg(checksum, 0, 16));
	updateLog(QString("%1").arg('\n'));
}
