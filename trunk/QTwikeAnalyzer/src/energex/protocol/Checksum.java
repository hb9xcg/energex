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
		for(int i=2; i<data.size()-3; i++) {
			byte checkByte = data.at(i);
			checksum ^= checkByte;
		}
		if( checksum == 0) {
			eChecksum = EChecksum.VALID;
		}
		
		return eChecksum;
	}
}
