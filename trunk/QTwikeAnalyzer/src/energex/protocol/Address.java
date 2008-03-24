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


public class Address {
	static final short BATTERIE1 = 0x31;
	static final short BATTERIE2 = 0x32;
	static final short BATTERIE3 = 0x34;
	static final short BROADCAST = -1;
	
	Map<Short,String> address2Desc = new HashMap<Short, String>();
	
	public Address() {
		address2Desc.put(BATTERIE1, "Batterie 1");
		address2Desc.put(BATTERIE2, "Batterie 2");
		address2Desc.put(BATTERIE3, "Batterie 3");
		address2Desc.put(BROADCAST, "Broadcast");
	}
	
	public boolean isAddress(byte address) {
		return (address==BROADCAST) || 
				 (address==BATTERIE1) || 
				 (address==BATTERIE2) ||
				 (address==BATTERIE3);
	}
	
	public String decodeAddress(byte data) {
		short address = data;
		String description = address2Desc.get(address);
		
		if(description == null) {
			description = "Unknown";
		}
		return description;		
	}
}
