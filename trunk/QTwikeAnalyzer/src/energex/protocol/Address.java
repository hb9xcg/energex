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
