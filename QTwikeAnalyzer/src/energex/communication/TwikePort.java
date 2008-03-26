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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.Enumeration;
import gnu.io.CommPortIdentifier;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.UnsupportedCommOperationException;

public class TwikePort {
	BufferedReader is = null;
	PrintStream    os = null;
	SerialPort port = null;
	CommPortIdentifier portId = null;
	boolean opened = false;

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

	public void open() throws IOException, UnsupportedCommOperationException {
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
		  is = new BufferedReader(new InputStreamReader(port.getInputStream()));
		} catch (IOException e) {
		  System.err.println("Can't open input stream: write-only");
		  is = null;
		}
		os = new PrintStream(port.getOutputStream(), true);

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
	}
	
	public void close() throws IOException {
		//
		// It is very important to close output/input streams as well as the port.
		// Otherwise Java, driver and OS resources are not released.
		//
		if (is != null) is.close();
		if (os != null) os.close();
		if (port != null) port.close();
		opened = false;
	}
	
	public boolean isOpen() {
		return opened;
		
	}
}
