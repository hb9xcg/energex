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

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import com.trolltech.qt.core.QCoreApplication;
import com.trolltech.qt.core.QDataStream;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QIODevice;
import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.gui.*;

import energex.communication.TwikePort;
import energex.protocol.DataInterface;
import energex.protocol.DataPacket;
import energex.protocol.OfflineDecoder;
import energex.protocol.OnlineDecoder;

public class TwikeAnalyzer extends QMainWindow implements DataInterface {

    Ui_TwikeAnalyzerClass ui = new Ui_TwikeAnalyzerClass();
    QDataStream binaryStream;
    TwikePort port;
    OnlineDecoder onlineDecoder;
    List<String> labels = new ArrayList<String>();
    QStandardItemModel model;
    boolean recordingPaused;
    
    
    public static void main(String[] args) {
        QApplication.initialize(args);
        
        QCoreApplication.setOrganizationName("Energex");
        QCoreApplication.setApplicationName("QTwikeAnalyzer");
		
        TwikeAnalyzer testTwikeAnalyzer = new TwikeAnalyzer();
        testTwikeAnalyzer.show();
        QApplication.exec();
    }
    
    public TwikeAnalyzer(){
        ui.setupUi(this);

        QStyle style = QStyleFactory.create("Plastique");
        QApplication.setStyle(style);     
        
        // restore current default
        QSettings settings = new QSettings();
		String storedPort = (String) settings.value("Port");
		
		if(storedPort!=null) {
			port = new TwikePort(storedPort);
			ui.statusBar.showMessage("Using serial port " + storedPort);
		}
		
		ui.actionRecord.setEnabled(!storedPort.isEmpty());
		ui.actionPlay.setEnabled(false);
		ui.actionPause.setEnabled(false);
		ui.actionStop.setEnabled(false);
		
		labels.add("Time");
	    labels.add("Paket");
	    labels.add("Address");
	    labels.add("Type");	    
	    labels.add("Message");
	    labels.add("Checksum");
		
		model = new QStandardItemModel(1, labels.size());
	    model.setHorizontalHeaderLabels(labels);
	    
	    ui.actionQuit.triggered.connect(QApplication.instance(), "quit()");
    }
    
	public TwikeAnalyzer(QWidget parent){
    	super(parent);
        ui.setupUi(this);
    }
	
	public void on_actionExit_triggered() {
    	try {
    		if(port!=null && port.isOpen()) {
    			port.close();
    		}
		} catch (Exception e) {
			e.printStackTrace();
		}
    }
	
	public void on_actionSettings_triggered() {
		SettingsDialog settingsDialog = new SettingsDialog(this);
		
    	if (settingsDialog.exec() != 0)
    	{ 
    		if(port!=null && port.isOpen()) {
    			try {
					port.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
    		}
    		port = new TwikePort(settingsDialog.getPort());
    		ui.statusBar.showMessage("Using serial port "+settingsDialog.getPortName());
    		ui.actionRecord.setEnabled(!settingsDialog.getPortName().isEmpty());	
    	}
	}
	
	public void on_actionRecord_triggered() {
    	try {
    		model.clear();
    		onlineDecoder = new OnlineDecoder(this);
			port.open(onlineDecoder);
			ui.actionRecord.setEnabled(false);
			ui.actionPlay.setEnabled(false);
			ui.actionPause.setEnabled(true);
			ui.actionStop.setEnabled(true);
			recordingPaused = false;
		} catch (Exception e) {
			e.printStackTrace();
		}
    }

    public void on_actionPause_triggered() {
    	try {
			ui.actionPlay.setEnabled(true);
			ui.actionPause.setEnabled(false);
			recordingPaused = true;
		} catch (Exception e) {
			e.printStackTrace();
		}
    }
	
    public void on_actionPlay_triggered() {
    	try {
			ui.actionPlay.setEnabled(false);
			ui.actionPause.setEnabled(true);
			recordingPaused = false;
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
			onlineDecoder = null;
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
    	OfflineDecoder decoder = new OfflineDecoder();
    	ui.logTable.setModel(decoder.decodeOffline(binaryStream));
    }
    
    public void on_actionAbout_triggered() {
    	AboutDialog aboutDialog = new AboutDialog(this);
		
    	aboutDialog.exec();
    }

	@Override
	public void updateData(DataPacket data) {
		QApplication.invokeLater(new UpdateTable(data));	
	}
	
	class UpdateTable implements Runnable
	{
		DataPacket data;
		
		public UpdateTable(DataPacket data) {
			this.data = data;
//			this.data.moveToThread(QApplication.instance().thread());
		}
		
		@Override
	    public void run()
	    { 
			data.updateModel(model);
	    	ui.logTable.setModel(model);
	    	if(!recordingPaused) {
	    		ui.logTable.scrollToBottom();
	    	}
	    }
	}


}
