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
package energex.gui;

import com.trolltech.qt.QThread;
import com.trolltech.qt.core.QByteArray;
import com.trolltech.qt.core.QDir;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QIODevice;
import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.core.QTime;
import com.trolltech.qt.gui.*;

import energex.communication.TwikePort;
import energex.downloader.Downloader;
import energex.tools.HexConverter;

public class DownloadDialog extends QDialog {

    Ui_DownloadDialog ui = new Ui_DownloadDialog();
    private Downloader downloader;
    private QThread downloadThread;
    private String fileName = null;
    private QTime time = new QTime();

    public Signal0 closeDownload = new Signal0();

    public static void main(String[] args) {
        QApplication.initialize(args);

        DownloadDialog testDownloadDialog = new DownloadDialog();
        testDownloadDialog.show();

        QApplication.exec();
    }

    public DownloadDialog() {
        ui.setupUi(this);
    }
    
    public DownloadDialog(QWidget parent, TwikePort port) {
    	super(parent);
        ui.setupUi(this);
        ui.progressBar.setMaximum(100);
        ui.progressBar.setValue(0);
        
        downloader = new Downloader(this, port);
        downloadThread = new QThread(downloader);
        downloadThread.setName("DownloadThread");
        downloadThread.setDaemon(true);
        
        downloader.moveToThread(downloadThread);
    	
    	downloader.receiveData.connect(this, "appendReceivedData(String)");
    	downloader.sendData.connect(this, "appendSentData(String)");
    	downloader.appendLog.connect(this, "appendLog(String)");
    	downloader.setProgress.connect(ui.progressBar, "setValue(int)");
    	
    	ui.buttonStart.clicked.connect(downloader, "start()");
    	closeDownload.connect(downloader, "close()");
    		
    	downloadThread.start();
    	
    	readSettings();
    	
    	openFile(fileName);
    	
    	ui.buttonStart.setEnabled( downloader.loaded() );
    }
    
    
    @SuppressWarnings("unused")
	private void on_buttonFile_clicked()
    {
		QFileDialog.Filter filter = new QFileDialog.Filter(tr("Intel Hex Files (*.hex *.ihex)"));
		fileName = QFileDialog.getOpenFileName(this, tr("Open Raw Logfile"), QDir.homePath(), filter );
	
		openFile(fileName);
		ui.buttonStart.setEnabled( downloader.loaded() );
    }
    
    private void openFile(String fileName) {
    	if (fileName.isEmpty()) {
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
		
		ui.projectFile.setText(fileName);	
		downloader.loadFile(file);
    }
    
    @SuppressWarnings("unused")
	private void on_buttonStart_clicked()
    {
    	time.start();
    }
    
    protected void closeEvent(QCloseEvent event)
    {
    	try {
	    	downloadThread.interrupt();
	    	closeDownload.emit();
	    	writeSettings();
	    	
			downloadThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
    	event.accept();
    }
    
    public void appendLog(String text) {
    	text = time.elapsed() + "ms:\t" + text;
    	ui.textLog.appendPlainText(text);
    }
    
    public void appendReceivedData(String data) {
    	data = "recv: " + data;
    	appendLog(data);
    }
    
    public void appendSentData(String data) {
    	data = "send: " + data;
    	appendLog(data);
    }
    
    public void setProgress(int percentage) {
    	ui.progressBar.setValue(percentage);
    }

	public void appendReceivedData(byte data) {
		QApplication.invokeLater(new Receiver(data));	
	}
	
	class Receiver implements Runnable
	{
		byte data;
		
		public Receiver(byte data) {
			this.data = data;
		}
		
		@Override
	    public void run()
	    { 
			String text = HexConverter.hex2string(data);
			text = "recv: " + text;
			appendLog(text);	    	
	    }
	}
	
	private void readSettings() {
    	QSettings settings = new QSettings();

        settings.beginGroup("DownloadDialog");
        restoreGeometry( (QByteArray) settings.value("geometry") );

        fileName = (String) settings.value("fileName", "");
        settings.endGroup();
    }
    
    private void writeSettings()
    {
    	QSettings settings = new QSettings();
    	settings.beginGroup("DownloadDialog");
        settings.setValue("geometry", saveGeometry());
        
        settings.setValue("fileName", fileName);
        
        settings.endGroup();
    }
}
