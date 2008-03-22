package energex.protocol;

import com.trolltech.qt.core.QByteArray;

public class Response {
	Request request = new Request();

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
		int idx;
		if(hasStatus(data)) {
			idx = 4;
		} else {
			idx = 3;
		}
		sCurrent = data.at(idx) & 0x000000ff;
		sCurrent <<= 8;
		sCurrent += (data.at(idx+1) & 0x000000ff);
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
		int idx;
		if(hasStatus(data)) {
			idx = 6;
		} else {
			idx = 5;
		}
		sVoltage = data.at(idx) & 0x000000ff;
		sVoltage <<= 8;
		sVoltage += data.at(idx+1) & 0x000000ff;
		float fVoltage = sVoltage;		 
		fVoltage /= 100;
		return Float.toString(fVoltage)+"V";
	}
	
	public String decodeParameter(QByteArray data) {
		int sParameter;
		int idx;
		
		if(hasStatus(data)) {
			idx = 8;
			
		} else {
			idx = 7;
		}
		
		sParameter = data.at(idx) & 0x000000ff;
		if(data.length() > idx+2) {
			sParameter <<= 8;
			sParameter += data.at(idx+1) & 0x000000ff;
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
