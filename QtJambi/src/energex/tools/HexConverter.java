package energex.tools;

public class HexConverter {
	static public String hex2string(byte[] data) {
		String str = new String();
		String hexNbr;
		for (int i=0; i<data.length; i++) {
			hexNbr = Integer.toHexString(data[i]);
			hexNbr = hexNbr.toUpperCase();
			if (hexNbr.length() == 1) {
				hexNbr = "0" + hexNbr;
			} else if (hexNbr.length() > 2) {
				hexNbr = hexNbr.substring(6, 8);
			}
			hexNbr += " ";
			str += hexNbr;
		}
		return str;		
	}
	
	static public String hex2string(byte data) {
		String hexNbr;		

		hexNbr = Integer.toHexString(data);
		hexNbr = hexNbr.toUpperCase();
		if (hexNbr.length() == 1) {
			hexNbr = "0" + hexNbr;
		} else if (hexNbr.length() > 2) {
			hexNbr = hexNbr.substring(6, 8);
		}
		return hexNbr;		
	}
	
	static public byte[] string2hex(String data) {
		byte[] buffer = new byte[data.length()/2];
		int j = 0;
		for (int i=0; i<data.length(); i+=2) {
			String str = new String();
			str += data.charAt(i);
			str += data.charAt(i+1);
			int number = Integer.parseInt(str, 16);
			buffer[j] = (byte) (number & 0xFF);
			j++;
		}
		return buffer;		
	}
}
