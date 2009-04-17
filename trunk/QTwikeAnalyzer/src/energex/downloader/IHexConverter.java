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

import com.trolltech.qt.core.QByteArray;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QTextStream;

import energex.tools.HexConverter;

public class IHexConverter {
	QByteArray binBuffer = new QByteArray();
	
	IHexConverter(QFile file) {
		open(file);
	}
	
	void open(QFile file) {
		QTextStream textStream = new QTextStream(file);
		
		String line = textStream.readLine();
		while (!line.isEmpty() && !line.equals(":00000001FF")) { 
			QByteArray lineBuffer = convertLine(line);
			binBuffer.append(lineBuffer);
			line = textStream.readLine();
		}
	}
	
	private QByteArray convertLine(String line) {
		String dataHex = line.substring(9, line.length()-2);
		byte[] dataTmp = HexConverter.string2hex(dataHex);
		QByteArray dataBin = new QByteArray(dataTmp);
		return dataBin;		
	}

	byte[] getBin() {
		return binBuffer.toByteArray();	
	}
}
