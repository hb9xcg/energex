package energex.gui;

import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.gui.QDialog;

public class AboutDialog extends QDialog {
	Ui_AboutDialogClass ui = new Ui_AboutDialogClass();
	QSettings settings = null;
	
	
	public AboutDialog(TwikeAnalyzer parent) {
		super(parent);
		ui.setupUi(this);
	}
}
