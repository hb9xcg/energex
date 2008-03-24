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

import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;

/**
 * Listener to handle all serial port events.
 * NOTE: It is typical that the SerialPortEventListener is implemented
 *       for the main class that is supposed to communicate with the
 *       device. That way it is easier to get access to the current
 *       state of communication.
 *
 *       However, for demonstration purposes this example implements a
 *       separate class.
 */ 
public class SerialListener implements SerialPortEventListener {

	/**
     * Handle serial events. Dispatches the event to event-specific
     * methods.
     * @param event The serial event
     */
    public void serialEvent(SerialPortEvent event){

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

    /**
     * Handle output buffer empty events.
     * NOTE: The reception is of this event is optional and not
     *       guaranteed by the API specification.
     * @param event The output buffer empty event
     */
    protected void outputBufferEmpty(SerialPortEvent event) {
        // Implement writing more data here
    }

    /**
     * Handle data available events.
     *
     * @param event The data available event
     */
    protected void dataAvailable(SerialPortEvent event) {
        // implement reading from the serial port here
    }

}
