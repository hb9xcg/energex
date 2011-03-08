/***************************************************************************
 *   Copyright (C) 2009 by Markus Walser                                   *
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
package energex.downloader;

import java.io.IOException;

import com.trolltech.qt.QThread;
import com.trolltech.qt.core.QEventLoop;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QObject;

import energex.communication.TwikePort;
import energex.communication.TwikeReceiver;
import energex.gui.DownloadDialog;
import energex.tools.HexConverter;
import gnu.io.UnsupportedCommOperationException;

public class Downloader extends QObject implements Runnable, TwikeReceiver {
	private TwikePort port;

	private enum EState {
		eReboot,
		eConnect,
		eDownload,
		eInvalid
	}
	private EState state = EState.eInvalid;
	
	private enum EProtocol {
		eCRC16,
		eSum
	}
	private EProtocol protocol = EProtocol.eCRC16;
	
	private byte reply;
	
	static final byte[] EXIT   = {'x','\r'};
	static final byte[] ESCAPE = {0x10,'d'};
	static final byte[] RETURN = {'\b','\r'};
	static final byte[] REBOOT = {'b','\r'};
	static final byte[] PASSWORD = {'A','V','R','U','B'};
	
	static final byte XMODEM_NUL = 0x00;
	static final byte XMODEM_SOH = 0x01;
	static final byte XMODEM_STX = 0x02;
	static final byte XMODEM_EOT = 0x04;
	static final byte XMODEM_ACK = 0x06;
	static final byte XMODEM_NAK = 0x15;
	static final byte XMODEM_CAN = 0x18;
	static final byte XMODEM_EOF = 0x1A;
	static final byte XMODEM_RWC = 0x43; //'C'
	
	static final int FRAME_LENGTH = 128;
	
	private byte[] dataBuffer;
	
	private QEventLoop eventLoop = null;
	private DownloadDialog dialog;
	
	public Signal1<String> receiveData = new Signal1<String>();
	public Signal1<String> sendData = new Signal1<String>();
	public Signal1<Integer> setProgress = new Signal1<Integer>();
	public Signal1<String> appendLog = new Signal1<String>();
	
	public Downloader(DownloadDialog dialog, TwikePort port)
	{
		this.dialog = dialog;
		this.port = port;
	}
	
	@Override
	public void receiveData(byte data) {
		
		dialog.appendReceivedData(data);
		
		switch(state) {
		case eConnect:
			if (data == 'C') {
				state = EState.eDownload;
			}
			break;
			
		case eDownload:
			if (data == XMODEM_ACK) {
				reply = XMODEM_ACK;
			} else {
				reply = XMODEM_NAK;
			}
			break;
			
		default:
			break;
		}
	}
	
	public void loadFile(QFile file) {
		IHexConverter data = new IHexConverter(file);
		dataBuffer = data.getBin();
	}
	
	public boolean loaded() {
		return dataBuffer != null;
	}

	public void start() {
		byte[] frameBuffer = new byte[FRAME_LENGTH + 5];
		byte[] crcBuffer = new byte[FRAME_LENGTH];

		try {
			setProgress.emit(0);
			state = EState.eReboot;
			
			if (port.isOpen()) {
				port.close();
			}			
			port.open();
			port.addReceiver(this);
			
			sendData(EXIT);
			port.flush();
			
			QThread.sleep(100);
			
			sendData(ESCAPE);
			port.flush();
			
			QThread.sleep(100);
			
			sendData(RETURN);
			port.flush();
						
			QThread.sleep(1000);
			
			sendData(REBOOT);			
			port.flush();
			
			QThread.sleep(200);
			port.close();	
			
			port.openBootloader();
			port.addReceiver(this);
			
			int loginCounter = 0;
			QThread.sleep(200);
			state = EState.eConnect;
			do {
				sendData(PASSWORD);
				QThread.sleep(200);
				if (++loginCounter>10) {
					appendLog.emit("Login failed.");
					return;
				}
			} while (state != EState.eDownload);
			

			int frameCounter = 0;
			int retransmissionCount = 0;
		    byte retransmissionNbr = 0;
		    byte PackNo = 1;
		    int crc = 0;
		    
		    do {
		    	reply = XMODEM_NUL;
		    	
		    	//Package No
		    	frameBuffer[0] = XMODEM_SOH;
		    	frameBuffer[1] = PackNo;
		    	frameBuffer[2] = (byte) (0xFF ^ (int)PackNo);
		    	
		    	//Data
		        for (int i=0; i<FRAME_LENGTH; i++) {
		        	byte dataByte = -1;
		        	int idx = frameCounter * FRAME_LENGTH + i;
		        	if (idx<dataBuffer.length) {
		        		dataByte = dataBuffer[idx];
		        	}
		        	frameBuffer[3+i] = dataByte;
		        	crcBuffer[i] = dataByte;
		        }
		        
		        //Checksum
		        switch(protocol) {
		        case eCRC16: 
		        	crc = Checksum.crc16(crcBuffer, FRAME_LENGTH, 0x1021, (short)0);
		        	break;
		        case eSum: 
		        	crc = Checksum.sumChecksum(crcBuffer, FRAME_LENGTH, 0x1021, (short)0);
		        	break;
		        default:
		        	crc = 0;
		        }
		        
		        frameBuffer[FRAME_LENGTH + 3] = (byte) (crc / 256);
		        frameBuffer[FRAME_LENGTH + 4] = (byte) (crc % 256);
		        
		        sendData(frameBuffer);
		        
		        setProgress.emit( (frameCounter * 100) / (dataBuffer.length / FRAME_LENGTH) );
		        
		        PackNo++;
		        
		        // wait for respond
		        boolean waiting = true;
		        for(int t=0; t<500 && waiting; t++) {
		        	QThread.sleep(1);
			        switch(reply) {
			        case XMODEM_NAK: //request resend
		                retransmissionCount++;
		                retransmissionNbr++;
		                waiting = false;
		                break;
			        case XMODEM_ACK: //send success, next
		                retransmissionCount = 0;
		                frameCounter++;
		                waiting = false;
		                break;
		            default:
		            	break;
			        }
		        }
		        
		        //timeout
		        if (reply == XMODEM_NUL) {
		        	retransmissionCount++;
		          	retransmissionNbr++;
		        }
		        
		        if (retransmissionCount > 3 || retransmissionNbr > 10) {
		        	appendLog.emit("Number of retries exceeded.");
		        	break;
		        }
		        
		        //Sent all data?
		        if (frameCounter > (dataBuffer.length / FRAME_LENGTH)) {
		        	//send finish
		        	byte[] endBuffer = new byte[1];
		        	endBuffer[0] = XMODEM_EOT;
		        	sendData(endBuffer);
		        	appendLog.emit("Download successful.");
		        	setProgress.emit(100);
		        	break;
		        }
		        
		    } while(true);
			
		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			return;
		} catch (UnsupportedCommOperationException e) {
			e.printStackTrace();
		}
	}

	private void sendData(byte[] data) throws IOException {
		port.sendData(data);
		String text = HexConverter.hex2string(data);
		sendData.emit(text);
	}

	@Override
	public void run() {
		try {
			eventLoop = new QEventLoop();
			eventLoop.exec();
		
			port.close();
			
			port = null;
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void close() {
		
		eventLoop.quit();
	}
}
