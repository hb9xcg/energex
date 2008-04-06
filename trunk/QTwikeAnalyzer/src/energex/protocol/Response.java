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

public class Response {
	Request request = new Request();
	private final static int STATUS1_INDEX      =  3;
	private final static int STATUS2_INDEX      =  4;
	private final static int CURRENT_HIGH_INDEX =  5;
	private final static int CURRENT_LOW_INDEX  =  6;
	private final static int VOLTAGE_HIGH_INDEX =  7;
	private final static int VOLTAGE_LOW_INDEX  =  8;
	private final static int PARAM_HIGH_INDEX   =  9;
	private final static int PARAM_LOW_INDEX    = 10;

	public String decodeResponse(short type, QByteArray data) {
		String response = new String();
		response += decodeCurrent(data);
		response += ", ";
		response += decodeVoltage(data);
		response += ", ";
		response += decodeParameter(data);
		response += request.getUnit(type);
		return response;
	}
	
	public String decodeCurrent(QByteArray data) {
		int sCurrent;

		sCurrent = data.at(CURRENT_HIGH_INDEX) & 0x000000ff;
		sCurrent <<= 8;
		sCurrent += (data.at(CURRENT_LOW_INDEX) & 0x000000ff);
		float fCurrent;
		if( sCurrent > 32767) {
			fCurrent = 65535-sCurrent;
		} else {
			fCurrent = sCurrent;
		}
		fCurrent /= 100;
		return Float.toString(fCurrent)+"A";
	}
	
	public String decodeVoltage(QByteArray data) {
		int sVoltage;

		sVoltage = data.at(VOLTAGE_HIGH_INDEX) & 0x000000ff;
		sVoltage <<= 8;
		sVoltage += data.at(VOLTAGE_LOW_INDEX) & 0x000000ff;
		float fVoltage = sVoltage;		 
		fVoltage /= 100;
		return Float.toString(fVoltage)+"V";
	}
	
	public String decodeParameter(QByteArray data) {
		int sParameter;
		
		sParameter = data.at(PARAM_HIGH_INDEX) & 0x000000ff;
		if(data.length() > PARAM_HIGH_INDEX+2) {
			sParameter <<= 8;
			sParameter += data.at(PARAM_LOW_INDEX) & 0x000000ff;
		}
		float fParameter = sParameter;
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
