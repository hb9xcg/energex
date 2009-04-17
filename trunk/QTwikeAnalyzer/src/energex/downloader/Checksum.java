package energex.downloader;

public class Checksum {

	// checksum of CRC16
	static public int crc16(byte[] buffer, Integer length, Integer  Mask, Short Init) throws InterruptedException {
		int  crc;
		int  i, t;
	
		crc = Init;
		for (i=0; i<length; i++) {
			t = buffer[i] * 256;
			crc = (crc ^ t);
			for (t=0; t<8; t++) {
				if ((crc & 0x8000) == 0x8000) {
					crc = (short) ((crc * 2) ^ Mask);
				} else {
					crc = (short) (crc * 2);
				}
				crc &= 0xFFFF;
			}
		}
		return crc;
	}

	//checksum of accumulate
	public static int sumChecksum(byte[] buf, Integer length, int Mask, Short Init) {
		int crc;
		int i;

		crc = Init;
		for (i=0; i<length; i++) {
		    crc += buf[i];
		    crc &= 0xFFFF;
		}	
		
		return crc;		
	}
}
