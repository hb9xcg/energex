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

import energex.communication.TwikePort;
import energex.communication.TwikeReceiver;
import gnu.io.UnsupportedCommOperationException;

public class Terminal implements TwikeReceiver{
	private TwikePort port;
	private Byte trigger = new Byte((byte) 0);
	
	public Terminal(TwikePort port) {
		this.port = port;
		open();
	}
	
	public void open() {
		try {
			port.close();
			port.open();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (UnsupportedCommOperationException e) {
			e.printStackTrace();
		}
	}
	
	public void close() {
		try {
			port.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public boolean isEscaped() throws IOException, InterruptedException {
		byte[] buffer = {'\n', '\r'};
		boolean escaped = false;
		port.sendData(buffer);
		
		trigger = 0;
		synchronized (trigger) {
			while (trigger.equals(63)) {
				trigger.wait(1000);
			}
		}
		
		return escaped;		
	}

	@Override
	public void receiveData(byte data) {
		if (trigger.equals(data)) {
			trigger = data;
			trigger.notify();
		}
	}
}
