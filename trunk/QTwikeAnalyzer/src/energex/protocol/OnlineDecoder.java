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
package energex.protocol;

import java.util.ArrayList;
import java.util.List;

import com.trolltech.qt.core.QByteArray;
import energex.communication.ReceiverInterface;

public class OnlineDecoder implements ReceiverInterface {
	List<String> labels = new ArrayList<String>();
	int currentRow;
	short currentRequest;
	EType currentType = EType.UNKNOWN;
	QByteArray currentData;
	Request requestDecoder = new Request();
	Checksum checksumDecoder = new Checksum();
	Address addressDecoder = new Address();
	Response responseDecoder = new Response();
	DataInterface table;
	DataPacket currentPacket;
	byte currentStartByte;
	byte currentAddressByte;
	byte currentTypeByte;
	
	enum EState {
		eMessage,
		eRequest,
		eResponse,
		eUnknown
	}
	
	enum EHeaderState {
		eStart,
		eType,
		eAddress
	}
	
	enum EType {
		MESSAGE,
		REQUEST,
		RESPONSE,
		UNKNOWN
	}
	
	EState eState;
	EHeaderState eHeaderState;
	
	public OnlineDecoder(DataInterface table) {
		labels.add("Time");
	    labels.add("Paket");
	    labels.add("Address");
	    labels.add("Type");	    
	    labels.add("Message");
	    labels.add("Checksum");
	    
	    eState = EState.eUnknown;
	    eHeaderState = EHeaderState.eStart;
	    
		currentRow = -1; // Skip first chunk
		
		this.table = table;
		
		//createTableModel(); 
	}
	
	private void unknowType() {
		currentPacket.setContentData("Unknown type");
	}

	private void decodeResponse(QByteArray data) {
		String description = responseDecoder.decodeResponse(currentRequest, data);
		description = "Response " + description;
		
		currentPacket.setContentData(description);
		
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);		
		currentPacket.setChecksumData(eChecksum);	
	}

	private void decodeRequest(QByteArray data) {
		currentRequest = requestDecoder.requestType(data);
		String description = requestDecoder.decodeRequest(data);
		description = "Request " + description;
		
		currentPacket.setContentData(description);
		
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);
		
		currentPacket.setChecksumData(eChecksum);
	}

	private void decodeMessage(QByteArray data) {
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);		
		currentPacket.setChecksumData(eChecksum);		
	}
	
	private boolean isStart(byte start) {
		return (start==0x10);
	}
	
	private boolean isType(byte type) {
		return type==0x42 || type==0x48 || type==0x24;
	}
	
	private void decodeContent(QByteArray data) {
		switch(currentType)
		{
		case MESSAGE:
			decodeMessage(data);
			break;
		case REQUEST:
			decodeRequest(data);
			break;
		case RESPONSE:
			decodeResponse(data);
			break;
		default:
			unknowType();
			break;
		}
	}

	private EType getType(byte byteType) {
		EType eType;

		switch(byteType) {
		case 0x42:
			eType = EType.REQUEST;
			break;
		case 0x48:
			eType = EType.RESPONSE;
			break;
		case 0x24:
			eType = EType.MESSAGE;
			break;
		default:
			eType = EType.UNKNOWN;
			break;
		}
		return eType;
	}

	@Override
	public void receiveData(byte data) {
		boolean newFrame = false;
		
		if(currentRow<0) {
			// Create buffer for initial waste chunk
			currentData = new QByteArray();
		}
		
		switch(eHeaderState) 
		{
		case eStart:
			if( isStart(data) ) {
				eHeaderState = EHeaderState.eAddress;
				currentStartByte = data;
			}
			break;
		case eAddress:
			if( addressDecoder.isAddress(data) ) {
				eHeaderState = EHeaderState.eType;
				currentAddressByte = data;
			} else {
				eHeaderState = EHeaderState.eStart;
			}
			break;
		case eType:
			if( isType(data) ) {
				eHeaderState = EHeaderState.eStart;
				currentTypeByte = data;
				newFrame = true;
			} 
			break;
		}
		
		currentData.append(data);
		
		if(newFrame) {
			if (currentRow >= 0) {
				processPacket();
			} else {
				// Skip first waste chunk
				currentRow++;
			}
		}	
	}
	
	public void processPacket() {
		currentData.chop(3); // Cut off the next header
		
		if(currentPacket!=null) {
			currentPacket.setRawData(currentData);
			decodeContent(currentData);
			table.updateData(currentPacket); // Moves ownership to main thread
		}		
		
		// Prepare next packet
		currentRow++;
		currentPacket = new DataPacket(currentRow);
		currentData   = new QByteArray();
		
		currentData.append(currentStartByte);
		currentData.append(currentAddressByte);
		currentData.append(currentTypeByte);	
		
		currentType = getType(currentTypeByte);
		
		currentPacket.setAddressData(currentAddressByte);
		currentPacket.setTypeData(currentType);
	}
}
