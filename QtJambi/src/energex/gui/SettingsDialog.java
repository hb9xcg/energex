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

import java.util.Enumeration;
import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.gui.QComboBox;
import com.trolltech.qt.gui.QDialog;

import gnu.io.CommPortIdentifier;

public class SettingsDialog extends QDialog {
	
	Ui_SettingsDialogClass ui = new Ui_SettingsDialogClass();
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
		settings.beginGroup("Settings");
		String storedPort = (String) settings.value("Port");
		settings.endGroup();

		if(storedPort != null) {
			int idx = ui.comboSerial.findText(storedPort);
			ui.comboSerial.setCurrentIndex(idx);
		}
	}
	
	public void on_buttonBox_accepted() {
		// store selection
		settings.beginGroup("Settings");
		settings.setValue("Port", ui.comboSerial.currentText());
		settings.endGroup();
	}
	
	public String getPortName() {
		return ui.comboSerial.currentText();
	}
	
	public CommPortIdentifier getPort() {
    	int idx = ui.comboSerial.currentIndex();
    	return (CommPortIdentifier) ui.comboSerial.itemData(idx);
    }
}
