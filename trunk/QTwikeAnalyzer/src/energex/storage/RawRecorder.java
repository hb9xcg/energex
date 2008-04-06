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

package energex.storage;

import com.trolltech.qt.core.QDataStream;
import com.trolltech.qt.core.QFile;

import energex.communication.TwikeReceiver;

public class RawRecorder implements TwikeReceiver {
	QDataStream out;
	QFile.OpenMode mode;
	QFile file;
	
	public RawRecorder() {
		file = new QFile("QTwikeAnalyzer.tmp");
        mode = new QFile.OpenMode();
        mode.set(QFile.OpenModeFlag.WriteOnly);
        mode.set(QFile.OpenModeFlag.Truncate);
        file.open(mode);
        out = new QDataStream(file);
	}
	
	public void saveAs(String name) {
        file.close();
        QFile.rename("QTwikeAnalyzer.tmp", name);
        file.open(mode);
        out = new QDataStream(file);
	}
	
	public void clear() {
		file.close();        
        file.open(mode);
        out = new QDataStream(file);
	}

	@Override
	public void receiveData(byte data) {
		out.writeByte(data);
	}
}
