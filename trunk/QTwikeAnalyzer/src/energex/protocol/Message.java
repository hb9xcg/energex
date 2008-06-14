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

import java.util.HashMap;
import java.util.Map;

import com.trolltech.qt.core.QByteArray;

public class Message {
	static Map<Short,String> typeToDesc = new HashMap<Short, String>();
	
	private static final short DRIVE_STATE     = 0x0011;
	private static final short GESCHWINDIGKEIT = 0x00FE;
	private static final short TAGESKILOMETER  = 0x00FF;
	
	private static final short COMMAND_INDEX   = 3;
	private static final short PARAMETER_INDEX = 5;
	
	public Message() {
		if(typeToDesc.isEmpty()) {
			initDescription();
		}
	}
	
	private void initDescription() {
		typeToDesc.put(GESCHWINDIGKEIT, "Geschwindigkeit");
		typeToDesc.put(TAGESKILOMETER,  "Tageskilometer");
		typeToDesc.put(DRIVE_STATE,     "Drive State");
	}
	
	public String decodeMessage(QByteArray data) {
		short messageType = getType(data);
		String description = typeToDesc.get(messageType);
		if( description == null) {
			description = "Unknown";
		} else {
			description += ": ";
		}
		
		switch(messageType) {
		case GESCHWINDIGKEIT:
			description += decodeSpeed(data);
			break;
		case TAGESKILOMETER:
			description += decodeDistance(data);
			break;
		case DRIVE_STATE:
			description += decodeDriveState(data);
			break;
		default:
			break;
		}		
		
		return description;		
	}

	public static short getType(QByteArray data) {
		int command1 = data.at(COMMAND_INDEX  ) & 0xff;
		int command2 = data.at(COMMAND_INDEX+1) & 0xff;
		int type = command1<<8;
		type += command2;

		return (short) type;
	}
	
	private String decodeSpeed(QByteArray data) {
		float fCurrent = DataType.decodeSigned16(data, PARAMETER_INDEX);
		fCurrent /= 100;
		return Float.toString(fCurrent)+"km/h";
	}
	
	private String decodeDistance(QByteArray data) {
		float fCurrent = DataType.decodeUnsigned16(data, PARAMETER_INDEX);
		fCurrent /= 100;
		return Float.toString(fCurrent)+"km";
	}
	
	private String decodeDriveState(QByteArray data) {
		int driveState = data.at(PARAMETER_INDEX+1) & 0xff;
		return DriveState.decodeDriveState(driveState);
	}
}
