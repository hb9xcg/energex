package energex.gui;

import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.gui.QDialog;

public class AboutDialog extends QDialog {
	AboutDialogClass ui = new AboutDialogClass();
	QSettings settings = null;
	
	
	@SuppressWarnings("unchecked")
	public AboutDialog(TwikeAnalyzer parent) {
		super(parent);
		ui.setupUi(this);
	}
}
