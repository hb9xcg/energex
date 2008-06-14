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
import java.util.ArrayList;
import java.util.List;

import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.*;

import energex.protocol.DataPacket;



public class FindDialog extends QDialog {
	FindDialogClass ui = new FindDialogClass();
	List<QStandardItem> foundItems;
	QTableView logTable;
	QStandardItemModel model;
	int idx = 0;
	QStandardItem lastItem;
	QBrush lastBackground;
	String textToFind;
	QSettings settings;
	
	@SuppressWarnings("unchecked")
	public FindDialog(TwikeAnalyzer parent) {
		super(parent);
		ui.setupUi(this);
		logTable = parent.ui.logTable;
		model = parent.model;
		
		settings = new QSettings();
		settings.beginGroup("Find");
		String findTime     = (String) settings.value("Time",     "false");
		String findPacket   = (String) settings.value("Packet",   "true");
		String findAddress  = (String) settings.value("Address",  "false");
		String findType     = (String) settings.value("Type",     "false");
		String findContent  = (String) settings.value("Content",  "true");
		String findChecksum = (String) settings.value("Checksum", "false");
		
		ui.editFind.setText( (String) settings.value("Text", "") );
		settings.endGroup();
		
		ui.checkBoxTime.setChecked( findTime.equals("true") );
		ui.checkBoxPaket.setChecked(findPacket.equals("true"));
		ui.checkBoxAddress.setChecked(findAddress.equals("true"));
		ui.checkBoxType.setChecked(findType.equals("true"));
		ui.checkBoxContent.setChecked(findContent.equals("true"));
		ui.checkBoxChecksum.setChecked(findChecksum.equals("true"));		
	}
	
	public void on_button_close_clicked() {
		// store selection
		settings.beginGroup("Find");
		settings.setValue("Time"    , ui.checkBoxTime.isChecked());
		settings.setValue("Packet"  , ui.checkBoxPaket.isChecked());
		settings.setValue("Address" , ui.checkBoxAddress.isChecked());
		settings.setValue("Type"    , ui.checkBoxType.isChecked());
		settings.setValue("Content" , ui.checkBoxContent.isChecked());
		settings.setValue("Checksum", ui.checkBoxChecksum.isChecked());
		
		settings.setValue("Text", ui.editFind.text());
		settings.endGroup();
		
		if (lastItem != null) {
			lastItem.setBackground(lastBackground);
		}
	}
	
	void on_button_find_clicked() {
		if (!ui.editFind.text().equals(textToFind) ) {
			textToFind = ui.editFind.text();
			Qt.MatchFlags flags = new Qt.MatchFlags();
			
			flags.set(Qt.MatchFlag.MatchContains);
			if (foundItems==null) {
				foundItems = new ArrayList<QStandardItem>();
			} else {
				foundItems.clear();
			}
			if(ui.checkBoxTime.isChecked()) {
				foundItems.addAll( model.findItems(textToFind, flags, DataPacket.COL_TIME));
			}
			if(ui.checkBoxPaket.isChecked()) {
				foundItems.addAll( model.findItems(textToFind, flags, DataPacket.COL_RAW));
			}
			if(ui.checkBoxAddress.isChecked()) {
				foundItems.addAll( model.findItems(textToFind, flags, DataPacket.COL_ADDRESS));
			}
			if(ui.checkBoxType.isChecked()) {
				foundItems.addAll( model.findItems(textToFind, flags, DataPacket.COL_TYPE));
			}
			if(ui.checkBoxContent.isChecked()) {
				foundItems.addAll( model.findItems(textToFind, flags, DataPacket.COL_CONTENT));
			}
			if(ui.checkBoxChecksum.isChecked()) {
				foundItems.addAll( model.findItems(textToFind, flags, DataPacket.COL_CHECKSUM));
			}
			idx=0;
		}
		
		if( !foundItems.isEmpty() ) {
			QStandardItem item = foundItems.get(idx);
			if( ++idx >= foundItems.size() ) {
				idx=0;
			}
			logTable.scrollTo(item.index());
			highlightItem(item);
		}		
	}
	
	void highlightItem(QStandardItem item) {
		if (lastItem != null) {
			lastItem.setBackground(lastBackground);
		}
		lastItem = item;
		lastBackground = item.background();
		QBrush brush = new QBrush();
        QColor color = new QColor();
        
        color.setRgb(220, 220, 0);
        
        brush.setColor(color);
        brush.setStyle(Qt.BrushStyle.SolidPattern);
        item.setBackground(brush);
	}
}
