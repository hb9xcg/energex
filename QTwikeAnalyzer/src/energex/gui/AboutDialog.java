package energex.gui;

import com.trolltech.qt.core.QSettings;
import com.trolltech.qt.gui.QDialog;

public class AboutDialog extends QDialog {
	Ui_AboutDialog ui = new Ui_AboutDialog();
	QSettings settings = null;
	
	
	@SuppressWarnings("unchecked")
	public AboutDialog(TwikeAnalyzer parent) {
		super(parent);
		ui.setupUi(this);
	}
}
