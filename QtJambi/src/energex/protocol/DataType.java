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

public class DataType {
	enum EUnit {
		eVoltage,
		eCurrent,
		eCharge,
		eTemperatur,
		ePower,
		eUnknown
	}
	
	public static String decodeUnit(EUnit eUnit) {
		switch(eUnit) {
		case eVoltage:
			return "V";
		case eCurrent:
			return "A";
		case eCharge:
			return "Ah";
		case eTemperatur:
			return "°C";
		case ePower:
			return "W";
		default:
			return ""; 
		}
	}
	
	public static float decodeSigned16(QByteArray data, int index) {
		int nData = data.at(index) & 0x000000ff;
		nData <<= 8;
		nData += (data.at(index+1) & 0x000000ff);
		float fData;
		if( nData > 32767) {
			fData = nData-65535;
		} else {
			fData = nData;
		}
		return fData;
	}
	
	public static float decodeUnsigned16(QByteArray data, int index) {
		int nData = data.at(index) & 0x000000ff;
		nData <<= 8;
		nData += (data.at(index+1) & 0x000000ff);
		return nData;
	}

	public static float decodeSigned8(QByteArray data, int index) {
		int nData = data.at(index) & 0x000000ff;
		return nData;
	}
}
