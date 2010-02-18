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

public class Checksum {
	enum EChecksum {
		VALID,
		INVALID
	}
	
	EChecksum decodeChecksum(QByteArray data) {
		EChecksum eChecksum = EChecksum.INVALID;
		short checksum = 0;
		for(int i=2; i<data.size(); i++) {
			byte checkByte = data.at(i);
			checksum ^= checkByte;
		}
		if( checksum == 0) {
			eChecksum = EChecksum.VALID;
		}
		
		return eChecksum;
	}
}
