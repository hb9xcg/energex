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
package energex.communication;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.TooManyListenersException;

import gnu.io.CommPortIdentifier;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;
import gnu.io.UnsupportedCommOperationException;

public class TwikePort implements SerialPortEventListener{
	private InputStream inputStream;
	private OutputStream outputStream;
	private SerialPort port = null;
	private CommPortIdentifier portId = null;
	private boolean opened = false;
	private ReceiverInterface receiver;
	
	
	public TwikePort(CommPortIdentifier portId) {
		this.portId = portId;
	}
	
	@SuppressWarnings("unchecked")
	public TwikePort(String portName) {
		// Get an enumeration of all ports known to JavaComm
		//
		Enumeration portIdentifiers = CommPortIdentifier.getPortIdentifiers();
		
		// Check each port identifier if 
		//   (a) it indicates a serial (not a parallel) port, and
		//   (b) matches the desired name.
		//
		while (portIdentifiers.hasMoreElements())
		{
		    CommPortIdentifier pid = (CommPortIdentifier) portIdentifiers.nextElement();
		    if( pid.getPortType() == CommPortIdentifier.PORT_SERIAL &&
		    	 pid.getName().equals(portName) ) {
		    	portId = pid;
		    	break;
		    }
		}
	}

	public void open(ReceiverInterface receiver) throws IOException, UnsupportedCommOperationException {
		if(portId == null)
		{
		    System.err.println("No serial port specifeid");
		    System.exit(1);
		}
		//
		// Use port identifier for acquiring the port
		//
		try {
		    port = (SerialPort) portId.open(
		        "QTwikeAnalyzer", // Name of the application asking for the port 
		        10000   // Wait max. 10 sec. to acquire port
		    );
		} catch(PortInUseException e) {
		    System.err.println("Port already in use: " + e);
		    System.exit(1);
		}
		// Now we are granted exclusive access to the particular serial
		// port. We can configure it and obtain input and output streams.
		//
		

		// Set all the params.  
		// This may need to go in a try/catch block which throws UnsupportedCommOperationException
		port.setSerialPortParams(
		    2400,
		    SerialPort.DATABITS_8,
		    SerialPort.STOPBITS_1,
		    SerialPort.PARITY_NONE);

		// Open the input Reader and output stream. The choice of a
		// Reader and Stream are arbitrary and need to be adapted to
		// the actual application. Typically one would use Streams in
		// both directions, since they allow for binary data transfer,
		// not only character data transfer.
		//
		try {
		  inputStream = port.getInputStream();
		} catch (IOException e) {
		  System.err.println("Can't open input stream: write-only");
		  inputStream = null;
		}
		outputStream = port.getOutputStream();

		// New Linuxes rely on UNICODE and it is possible you need to specify here the encoding scheme to be used
		// for example : 
		//		     os = new PrintStream(port.getOutputStream(), true, "ISO-8859-1");
		// will ensure that you sent 8 bits characters on your port. Don't know about a modem accepting
		// UNICODE data for its commands... 
		//
		// Actual data communication would happen here
		// performReadWriteCode();
		//
		opened = true;
		this.receiver = receiver;
		try {
			port.addEventListener(this);
		} catch (TooManyListenersException e) {
			e.printStackTrace();
		}
		port.notifyOnDataAvailable(true);
	}
	
	public void close() throws IOException {
		//
		// It is very important to close output/input streams as well as the port.
		// Otherwise Java, driver and OS resources are not released.
		//
		if (inputStream != null) { 
			inputStream.close();
		}
		if (outputStream != null) {
			outputStream.close();
		}
		if (port != null) { 
			port.close();
		}
		opened = false;
		receiver = null;
	}
	
	public boolean isOpen() {
		return opened;
	}

	@Override
	public void serialEvent(SerialPortEvent event) {
        //
        // Dispatch event to individual methods, to avoid a
        // cluttered, messy switch/case statement.
        //
        switch(event.getEventType()) {
            case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
                outputBufferEmpty(event);
                break;

            case SerialPortEvent.DATA_AVAILABLE:
                dataAvailable(event);
                break;

/* Other events, not implemented here ->
            case SerialPortEvent.BI:
                breakInterrupt(event);
                break;

            case SerialPortEvent.CD:
                carrierDetect(event);
                break;

            case SerialPortEvent.CTS:
                clearToSend(event);
                break;

            case SerialPortEvent.DSR:
                dataSetReady(event);
                break;

            case SerialPortEvent.FE:
                framingError(event);
                break;

            case SerialPortEvent.OE:
                overrunError(event);
                break;

            case SerialPortEvent.PE:
                parityError(event);
                break;
            case SerialPortEvent.RI:
                ringIndicator(event);
                break;
<- other events, not implemented here */

        }
		
	}

	private void outputBufferEmpty(SerialPortEvent event) {

	}

	private void dataAvailable(SerialPortEvent event) {
		byte data = 0;
    	try{
            while (inputStream.available() > 0) {
            	data = (byte) inputStream.read();
            	if(receiver!=null) {
            		receiver.receiveData(data);
            	}
            }
        }catch(IOException e){
            System.out.print(e);
        }
	}
}
