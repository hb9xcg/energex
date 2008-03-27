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
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QBrush;
import com.trolltech.qt.gui.QColor;
import com.trolltech.qt.gui.QStandardItem;
import com.trolltech.qt.gui.QStandardItemModel;

import energex.communication.ReceiverInterface;
import energex.protocol.Checksum.EChecksum;

public class OnlineDecoder implements ReceiverInterface {
	List<String> labels = new ArrayList<String>();
	QStandardItemModel model;
	int currentRow;
	short currentRequest;
	EType currentType;
	QByteArray currentData;
	Request requestDecoder = new Request();
	Checksum checksumDecoder = new Checksum();
	Address addressDecoder = new Address();
	Response responseDecoder = new Response();
	DataInterface table;
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
		int nbrOfColumns = labels.size();
		model = new QStandardItemModel(1, nbrOfColumns);
	    model.setHorizontalHeaderLabels(labels);
	    this.table = table;
	    table.updateData(model);
	}    
	
	private void unknowType() {
		QStandardItem msgItem   = new QStandardItem( "Unknown type");
		model.setItem(currentRow, 4, msgItem);
	}
	
	private void addChecksum(EChecksum eChecksum) {
		QStandardItem chksumItem = new QStandardItem( eChecksum.toString());
		
		QBrush brush = new QBrush();
        QColor color = new QColor();
        if(eChecksum==EChecksum.INVALID) {
        	color.setRed(180);
        } else {
        	color.setGreen(180);
        }
        brush.setColor(color);
        brush.setStyle(Qt.BrushStyle.SolidPattern);
        chksumItem.setBackground(brush);
        
        chksumItem.setTextAlignment(Qt.AlignmentFlag.AlignCenter);
        
        model.setItem(currentRow, 5, chksumItem);
	}

	private void decodeResponse(QByteArray data) {
		String description = responseDecoder.decodeResponse(currentRequest, data);
		description = "Response " + description;
		
		QStandardItem requestItem = new QStandardItem(description);
		model.setItem(currentRow, 4, requestItem);
		
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);		
		addChecksum(eChecksum);	
	}

	private void decodeRequest(QByteArray data) {
		currentRequest = requestDecoder.requestType(data);
		String description = requestDecoder.decodeRequest(data);
		description = "Request " + description;
		
		QStandardItem requestItem = new QStandardItem(description);
		model.setItem(currentRow, 4, requestItem);
		
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);
		
		addChecksum(eChecksum);
	}

	private void decodeMessage(QByteArray data) {
		
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);		
		addChecksum(eChecksum);		
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

	private String byteToHexString(byte input) {
		String hexString = Integer.toHexString(input);
		if(hexString.length()!=2)
		{
			if( hexString.length()==1) {
				hexString = "0" + hexString;
			} else {
				hexString = hexString.substring(6, 8);
			}
		}
		return hexString;
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
				// Skip first chunk
				currentRow++;
			}
		}	
	}
	
	public void processPacket() {
		// Ignore first chunk
		String rawPacket = new String();
		currentData.chop(3);
		for(int idx=0; idx<currentData.size(); idx++) {
			rawPacket += byteToHexString(currentData.at(idx)) + " ";
		}
		// Dump last raw packet...
		QStandardItem packetItem = new QStandardItem(rawPacket);
		model.setItem(currentRow, 1, packetItem);
		
		decodeContent(currentData);
		
		table.updateData(model);
		
		
		// Prepare next packet
		currentData = new QByteArray();
		currentRow++;
		currentData.append(currentStartByte);
		currentData.append(currentAddressByte);
		currentData.append(currentTypeByte);	
		
		QStandardItem timeItem   = new QStandardItem(QTime.currentTime().toString());
		QStandardItem addrItem   = new QStandardItem(addressDecoder.decodeAddress(currentAddressByte));

		model.setItem(currentRow, 0, timeItem);	
		model.setItem(currentRow, 2, addrItem);

		currentType = getType(currentTypeByte);
		QStandardItem typeItem   = new QStandardItem(currentType.toString());
		model.setItem(currentRow, 3, typeItem);
	}
}
