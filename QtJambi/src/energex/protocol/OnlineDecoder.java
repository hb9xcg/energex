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
import com.trolltech.qt.core.QTime;

import energex.communication.TwikeReceiver;
import energex.protocol.Type.EType;

public class OnlineDecoder implements TwikeReceiver {
	List<String> labels = new ArrayList<String>();
	int currentRow;
	short currentRequest;
	EType currentType = EType.UNKNOWN;
	QByteArray currentData;
	private QTime time = new QTime();
	Request requestDecoder = new Request();
	Checksum checksumDecoder = new Checksum();
	Address addressDecoder = new Address();
	Response responseDecoder = new Response();
	Message messageDecoder = new Message();
	DataInterface table;
	DataPacket currentPacket;
	byte currentStartByte;
	byte currentAddressByte;
	byte currentTypeByte;
	
	enum EHeaderState {
		eStart,
		eType,
		eAddress
	}

	EHeaderState eHeaderState;
	
	public OnlineDecoder(DataInterface table) {
		labels.add("Time");
	    labels.add("Paket");
	    labels.add("Address");
	    labels.add("Type");	    
	    labels.add("Message");
	    labels.add("Checksum");
	    
	    eHeaderState = EHeaderState.eStart;
	    
		currentRow = -1; // Skip first chunk
		
		this.table = table;
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
		String description = messageDecoder.decodeMessage(data);
		
		currentPacket.setContentData(description);
		
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);		
		currentPacket.setChecksumData(eChecksum);		
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
	
	public void start() {
		time.start();
	}
	
	public void processPacket() {
		currentData.chop(3); // Cut off the next header
		
		if(currentPacket!=null) {
			currentPacket.setTimeData(time);
			currentPacket.setRawData(currentData);
			currentData = Frame.unstuffing(currentData);
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
		
		currentType = Type.getType(currentTypeByte);
		
		currentPacket.setAddressData(currentAddressByte);
		currentPacket.setTypeData(currentType);
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
			if( Frame.matches(data) ) {
				eHeaderState = EHeaderState.eAddress;
				currentStartByte = data;
			}
			break;
		case eAddress:
			if( Address.matches(data) ) {
				eHeaderState = EHeaderState.eType;
				currentAddressByte = data;
			} else {
				eHeaderState = EHeaderState.eStart;
			}
			break;
		case eType:
			if( Type.matches(data) ) {
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
}
