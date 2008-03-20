package energex;

import com.trolltech.qt.core.QDataStream;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QIODevice;
import com.trolltech.qt.gui.*;

import energex.protocol.Decoder;

public class TwikeAnalyzer extends QMainWindow{

    Ui_TwikeAnalyzerClass ui = new Ui_TwikeAnalyzerClass();
    QDataStream binaryStream;
    
    public static void main(String[] args) {
        QApplication.initialize(args);
        TwikeAnalyzer testTwikeAnalyzer = new TwikeAnalyzer();
        testTwikeAnalyzer.show();
        QApplication.exec();
    }
    
    public TwikeAnalyzer(){
        ui.setupUi(this);
    }
    
    public TwikeAnalyzer(QWidget parent){
    	super(parent);
        ui.setupUi(this);
    }
    
    public void on_openButton_clicked() {
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
