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
#include "protocol.h"
#include "uart.h"
//#include "os_thread.h"
#include "battery.h"

#define START_DATA_IDX		5
#define OFF			0
#define ON			1

static uint8_t address;
static uint8_t command;
static uint8_t parameter;
static uint8_t length;
static uint8_t data[8];
static uint8_t checksum;

typedef enum
{
	eExpectStart,
	eExpectAddress,
	eExpectCommand,
	eExpectData,
	eExpectChecksum
} EPacketState;

static EPacketState eState = eExpectStart;
static uint8_t idx = 0;
static uint8_t frameDetection = ON;

// Prototypes
static void receivePacket(void);
static void receiveData(void);
static void transmitData(void);
static void transmitGroup(void);
static uint8_t frame_stuffing(uint8_t packet[], uint8_t length);

void protocol_receive_byte(uint8_t character)
{
	switch(eState)
	{
	case eExpectStart:
		switch(character)
		{
		case FRAME:
			eState = eExpectAddress;
			break;
		default:
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
		if( frameDetection && idx>0 && data[idx-1] == FRAME)
		{
			if(character == FRAME)
			{
				// FRAME character was doubled:
				// -> add one frame character to the data buffer.
				frameDetection = OFF;						
			}
			else
			{
				// A single FRAME character detected:
				// -> Reset state machine to start condition.
				address = character;
				eState = eExpectCommand;
				break;
			}
		}
		else
		{				
			frameDetection = ON;
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

void receivePacket(void)
{
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

void receiveData(void)
{
	uint16_t value;
	
	value = data[3] | (data[2]<<8);
	
	setParameterValue(parameter, value);
}

void transmitData(void)
{
	uint8_t packet[11];
	int16_t value;
	int8_t checksum=0, i;
	
	value = getParameterValue(parameter);
	
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
	
	length = frame_stuffing(packet, 8);
	
	uart_write( packet, length);
}

void transmitGroup(void)
{
	uint8_t packet[18];
	int16_t value, current, voltage;
	int8_t checksum=0, i;
	
	value   = getParameterValue(parameter);
	voltage = getParameterValue(TOTAL_SPANNUNG);
	current = getParameterValue(IST_STROM);
	
	packet[ 0] = FRAME;
	packet[ 1] = address;
	packet[ 2] = TRM_DATA;
	packet[ 3] = 0;
	packet[ 4] = 0;
	packet[ 5] = current >> 8;
	packet[ 6] = current & 0xff;
	packet[ 7] = voltage >> 8;
	packet[ 8] = voltage & 0xff;
	packet[ 9] = value >> 8;
	packet[10] = value & 0xff;
	
	for(i=2; i<=10; i++)
		checksum ^= packet[i];
	
	packet[11] = checksum;
	
	length = frame_stuffing(packet, 12);
	
	uart_write( packet, length);
}

uint8_t frame_stuffing(uint8_t packet[], uint8_t length)
{
	int8_t idxFrom, idxTo;
	for(idxFrom=START_DATA_IDX; idxFrom<length; idxFrom++)
	{
		if(packet[idxFrom] == FRAME)
		{
			for(idxTo=length; idxTo>idxFrom; idxTo--)
			{
				packet[idxTo] = packet[idxTo-1];
			}
			length++;
			idxFrom++;
		}
	}
	return length;
}
