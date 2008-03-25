package energex.gui;

import java.util.Enumeration;
import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.gui.QComboBox;
import com.trolltech.qt.gui.QDialog;

import energex.gui.Ui_Settings;
import gnu.io.CommPortIdentifier;

public class SettingsDialog extends QDialog {
	
	Ui_Settings ui = new Ui_Settings();
	QSettings settings = null;
	
	public SettingsDialog() {
		ui.setupUi(this);	
	}
	
	@SuppressWarnings("unchecked")
	public SettingsDialog(TwikeAnalyzer parent) {
		super(parent);
		ui.setupUi(this);
		
		ui.comboSerial.setInsertPolicy(QComboBox.InsertPolicy.InsertAlphabetically);
		
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
		    if(pid.getPortType() == CommPortIdentifier.PORT_SERIAL ) {
		    	ui.comboSerial.addItem(pid.getName(), pid);
		    }
		}
		
		// restore current default
		settings = new QSettings();
		String storedPort = (String) settings.value("Port");

		if(storedPort != null) {
			int idx = ui.comboSerial.findText(storedPort);
			ui.comboSerial.setCurrentIndex(idx);
		}
	}
	
	public void on_buttonBox_accepted() {
		// store selection
		settings.setValue("Port", ui.comboSerial.currentText());
	}
	
	public String getPortName() {
		return ui.comboSerial.currentText();
	}
	
	public CommPortIdentifier getPort() {
    	int idx = ui.comboSerial.currentIndex();
    	return (CommPortIdentifier) ui.comboSerial.itemData(idx);
    }
}
