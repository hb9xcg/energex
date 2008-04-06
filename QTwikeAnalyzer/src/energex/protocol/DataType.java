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
			fData = 65535-nData;
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
