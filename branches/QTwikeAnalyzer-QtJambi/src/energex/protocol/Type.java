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

public class Type {
	private final static byte REQUEST  = 0x42;
	private final static byte RESPONSE = 0x48;
	private final static byte MESSAGE  = 0x24;
	private final static byte REPLY    = 0x22;
	private final static String descRequest  = "Request";
	private final static String descResponse = "Response";
	private final static String descMessage  = "Message";
	private final static String descReply    = "Reply";
	
	enum EType {
		MESSAGE,
		REPLY,
		REQUEST,
		RESPONSE,
		UNKNOWN
	}
	
	public static boolean matches(byte data) {
		return (data==REPLY    || 
				 data==MESSAGE  ||
				 data==RESPONSE || 
				 data==REQUEST   );
	}
	
	public static String decodeType(QByteArray data) {
		return decodeType(data.at(2));
	}
	
	public static String decodeType(byte data) {
		String description = null;
		switch(data) {
		case REPLY:
			description = descReply;
			break;
		case MESSAGE:
			description = descMessage;
			break;
		case RESPONSE:
			description = descResponse;
			break;
		case REQUEST:
			description = descRequest;
			break;
		default:
			description = "Unknown";
		}
		return description;
	}
	
	public static EType getType(byte byteType) {
		EType eType;

		switch(byteType) {
		case REQUEST:
			eType = EType.REQUEST;
			break;
		case RESPONSE:
			eType = EType.RESPONSE;
			break;
		case MESSAGE:
			eType = EType.MESSAGE;
			break;
		case REPLY:
			eType = EType.REPLY;
			break;
		default:
			eType = EType.UNKNOWN;
			break;
		}
		return eType;
	}

	public static String decodeType(EType currentType) {
		String description = null;
		
		switch(currentType) {
		case REQUEST:
			description = descRequest;
			break;
		case RESPONSE:
			description = descResponse;
			break;
		case MESSAGE:
			description = descMessage;
			break;
		case REPLY:
			description = descReply;
			break;
		}
		return description;
	}
}
