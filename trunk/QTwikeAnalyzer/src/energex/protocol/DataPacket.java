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

import com.trolltech.qt.core.QByteArray;
import com.trolltech.qt.core.QTime;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QBrush;
import com.trolltech.qt.gui.QColor;
import com.trolltech.qt.gui.QStandardItem;
import com.trolltech.qt.gui.QStandardItemModel;

import energex.protocol.Checksum.EChecksum;
import energex.protocol.OnlineDecoder.EType;

public class DataPacket {
	QStandardItem timeItem;
	QStandardItem rawItem;
	QStandardItem addressItem;
	QStandardItem typeItem;
	QStandardItem contentItem;
	QStandardItem checksumItem;
	int row;
	
	private static Address addressDecoder = new Address();
	
	public DataPacket(int row) {
		this.row = row;
	}
	
	public void setTimeData(QTime time) {
		timeItem = new QStandardItem( time.elapsed() + "ms");
	}
	
	public void setRawData(QByteArray raw) {
		String rawString = new String();
		for(int idx=0; idx<raw.size(); idx++) {
			rawString += byteToHexString(raw.at(idx)) + " ";
		}
		// Dump last raw packet...
		rawItem = new QStandardItem(rawString);
	}
	
	public void setAddressData(byte address) {
		addressItem = new QStandardItem(addressDecoder.decodeAddress(address));
	}

	public void setTypeData(EType type) {
		typeItem = new QStandardItem(type.toString());
	}

	public void setContentData(String content) {
		contentItem = new QStandardItem(content);
	}	
	
	public void setChecksumData(EChecksum checksum) {
		checksumItem = new QStandardItem( checksum.toString());
		
		QBrush brush = new QBrush();
        QColor color = new QColor();
        if(checksum==EChecksum.INVALID) {
        	color.setRed(180);
        } else {
        	color.setGreen(180);
        }
        brush.setColor(color);
        brush.setStyle(Qt.BrushStyle.SolidPattern);
        checksumItem.setBackground(brush);
        
        checksumItem.setTextAlignment(Qt.AlignmentFlag.AlignCenter);
	}
	
	public void updateModel(QStandardItemModel model) {
		model.setItem(row, 0, timeItem);
		model.setItem(row, 1, rawItem);
		model.setItem(row, 2, addressItem);
		model.setItem(row, 3, typeItem);
		model.setItem(row, 4, contentItem);
		model.setItem(row, 5, checksumItem);
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

	public int getRow() {
		return row;		
	}
}
