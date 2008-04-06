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

import energex.protocol.DataType.EUnit;

public class Response {
	EUnit eType = EUnit.eUnknown;
	private final static int STATUS_INDEX    =  3;
	private final static int CURRENT_INDEX   =  5;
	private final static int VOLTAGE_INDEX   =  7;
	private final static int PARAMETER_INDEX =  9;

	public String decodeResponse(short type, QByteArray data) {
		String response = new String();
		
		eType = Request.getUnit(type);
		
		response += decodeCurrent(data);
		response += ", ";
		response += decodeVoltage(data);
		response += ", ";
		response += decodeParameter(data);
		response += DataType.decodeUnit(eType);
		return response;
	}
	
	public String decodeCurrent(QByteArray data) {
		float fCurrent = DataType.decodeSigned16(data, CURRENT_INDEX);
		fCurrent /= 100;
		return Float.toString(fCurrent)+"A";
	}
	
	public String decodeVoltage(QByteArray data) {
		float fVoltage = DataType.decodeUnsigned16(data, VOLTAGE_INDEX); 
		fVoltage /= 100;
		return Float.toString(fVoltage)+"V";
	}
	
	public String decodeParameter(QByteArray data) {
		float fParameter;
		
		int length = data.length();
		if(length <= PARAMETER_INDEX+2) {
			return "corrupt";
		}

		switch(eType) {
		case eVoltage:
			fParameter = DataType.decodeUnsigned16(data, PARAMETER_INDEX);
			break;
		case eCurrent:
			fParameter = DataType.decodeSigned16(data, PARAMETER_INDEX);
			break;
		case ePower:
			fParameter = DataType.decodeSigned16(data, PARAMETER_INDEX);
			break;
		case eCharge:
			fParameter = DataType.decodeSigned16(data, PARAMETER_INDEX);
			break;
		case eTemperatur:
			fParameter = DataType.decodeSigned16(data, PARAMETER_INDEX);
			break;
		default:
			fParameter = DataType.decodeSigned16(data, PARAMETER_INDEX);
		}
		
		fParameter /= 100;
		return Float.toString(fParameter);
	}
	
	
	private boolean hasStatus(QByteArray data) {
		short status = data.at(3);
		status &= 0xff;
		if(status == 0x80) {
			return true;
		} else {
			return false;
		}
	}
}
