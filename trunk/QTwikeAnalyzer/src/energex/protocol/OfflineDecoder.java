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
import com.trolltech.qt.core.QDataStream;
import com.trolltech.qt.core.QTime;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QBrush;
import com.trolltech.qt.gui.QColor;
import com.trolltech.qt.gui.QStandardItem;
import com.trolltech.qt.gui.QStandardItemModel;

import energex.exceptions.EndOfFileException;
import energex.protocol.Checksum.EChecksum;
import energex.protocol.Address;

public class OfflineDecoder {
	List<String> labels = new ArrayList<String>();
	QStandardItemModel model;
	int currentRow;
	EType currentType = EType.UNKNOWN;
	short currentRequest;
	QByteArray currentData;
	QTime time = new QTime();
	Request requestDecoder = new Request();
	Checksum checksumDecoder = new Checksum();
	Address addressDecoder = new Address();
	Response responseDecoder = new Response();
	Message messageDecoder = new Message();
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
	
	public OfflineDecoder(DataInterface table) {
		labels.add("Time");
	    labels.add("Paket");
	    labels.add("Address");
	    labels.add("Type");	    
	    labels.add("Message");
	    labels.add("Checksum");
	    
	    eState = EState.eUnknown;
	    eHeaderState = EHeaderState.eStart;
	    this.table = table;
	    time.start();
	}
	
	public OfflineDecoder() {
		labels.add("Time");
	    labels.add("Paket");
	    labels.add("Address");
	    labels.add("Type");	    
	    labels.add("Message");
	    labels.add("Checksum");
	    
	    eState = EState.eUnknown;
	    time.start();
	}
	
	public QStandardItemModel decodeOffline(QDataStream input) {
		try {
			currentRow = -1;
			int nbrOfColumns = labels.size();
			model = new QStandardItemModel(1, nbrOfColumns);
			
			do {
				decodeHeader(input);				
			} while(!input.atEnd());	    
		} catch (Exception e) {
			model.removeRow(currentRow);
		}
	    
	    model.setHorizontalHeaderLabels(labels);
	    
		return model;
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
		String description = messageDecoder.decodeMessage(data);
		
		QStandardItem messageItem = new QStandardItem(description);
		model.setItem(currentRow, 4, messageItem);
		
		Checksum.EChecksum eChecksum = checksumDecoder.decodeChecksum(data);		
		addChecksum(eChecksum);
	}
	
	
	private boolean isStart(byte start) {
		return (start==0x10);
	}
	
	private boolean isType(byte type) {
		return type==0x42 || type==0x48 || type==0x24;
	}

	private EType decodeHeader(QDataStream input) throws Exception {
		Byte start, address, type;
		if(currentRow<0) {
			currentData = new QByteArray();
		}
		do {
			do {
				do {
					start = input.readByte();
					currentData.append(start);					
					if(input.atEnd())
						throw new EndOfFileException("Reached input stream end");
				} while( !isStart(start) );
				address = input.readByte();
				currentData.append(address);
			} while(!addressDecoder.isAddress(address));
			type = input.readByte();
			currentData.append(start);
		} while(!isType(type));
		
		// Ignore first chunk
		if(currentRow>=0) {
			String rawPacket = new String();
			for(int idx=0; idx<currentData.size()-3; idx++) {
				rawPacket += byteToHexString(currentData.at(idx)) + " ";
			}
			// Dump last raw packet...
			QStandardItem packetItem = new QStandardItem(rawPacket);
			model.setItem(currentRow, 1, packetItem);
			currentData.chop(3);
			decodeContent(currentData);
		}
		currentData = new QByteArray();
		
		currentRow++;
		currentType = getType(type);
		
		currentData.append(start);
		currentData.append(address);
		currentData.append(type);
		
		QStandardItem timeItem   = new QStandardItem(time.elapsed() + "ms");
		QStandardItem addrItem   = new QStandardItem(addressDecoder.decodeAddress(address));

		model.setItem(currentRow, 0, timeItem);	
		model.setItem(currentRow, 2, addrItem);
	
		QStandardItem typeItem   = new QStandardItem(currentType.toString());
		model.setItem(currentRow, 3, typeItem);
		
		return currentType;
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
}
