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
package energex.gui;

import com.trolltech.qt.core.QDataStream;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QIODevice;
import com.trolltech.qt.gui.*;

import energex.communication.TwikePort;
import energex.protocol.Decoder;

public class TwikeAnalyzer extends QMainWindow{

    Ui_TwikeAnalyzerClass ui = new Ui_TwikeAnalyzerClass();
    QDataStream binaryStream;
    
    TwikePort port = new TwikePort("/dev/ttyS0");
    
    public static void main(String[] args) {
        QApplication.initialize(args);
        TwikeAnalyzer testTwikeAnalyzer = new TwikeAnalyzer();
        testTwikeAnalyzer.show();
        QApplication.exec();
    }
    
    public TwikeAnalyzer(){
        ui.setupUi(this);
        QStyle style = QStyleFactory.create("Plastique");
        QApplication.setStyle(style);
    }
    
	public TwikeAnalyzer(QWidget parent){
    	super(parent);
        ui.setupUi(this);
    }
	
	public void on_actionRecord_triggered() {
    	try {
			port.open();
			ui.actionRecord.setEnabled(false);
			ui.actionPlay.setEnabled(false);
			ui.actionPause.setEnabled(true);
			ui.actionStop.setEnabled(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
    }

    public void on_actionPause_triggered() {
    	try {
			ui.actionPlay.setEnabled(true);
			ui.actionPause.setEnabled(false);
		} catch (Exception e) {
			e.printStackTrace();
		}
    }
	
    public void on_actionPlay_triggered() {
    	try {
			ui.actionPlay.setEnabled(false);
			ui.actionPause.setEnabled(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
    }
    
    public void on_actionStop_triggered() {
    	try {
			port.close();
			ui.actionRecord.setEnabled(true);
			ui.actionPlay.setEnabled(false);
			ui.actionPause.setEnabled(false);
			ui.actionStop.setEnabled(false);
		} catch (Exception e) {
			e.printStackTrace();
		}
    }
    
    public void on_actionOpen_triggered() {
    	String fileName = null;
    	fileName = QFileDialog.getOpenFileName(this, tr("Open Image"), "/home/markus/" );
//    	fileName = QFileDialog.getOpenFileName(this, tr("Open log file"), "~/", tr("Log Files (*.log)"), null);
        if ( fileName.isEmpty()) {
        	return;
        }

    	QFile file = new QFile(fileName);
    	boolean done = file.open(QIODevice.OpenModeFlag.ReadOnly);
    	if (!done) {
    		String strError = new String("Cannot read file ");
    		strError += file.errorString();
            QMessageBox.warning(this, tr("Application"), strError);
            return;
    	}
    	
    	QDataStream binaryStream = new QDataStream( file ); // we will serialize the data into the file
    	Decoder decoder = new Decoder();
    	ui.logTable.setModel(decoder.decode(binaryStream));
    }
}
