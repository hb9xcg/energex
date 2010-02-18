#include <QtGui/QApplication>
#include <QTextCodec>
#include "twikeanalyzer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    a.setOrganizationName("Energex");
    a.setApplicationName("QTwikeAnalyzer");

    QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );

    TwikeAnalyzer w;
    w.show();

    return a.exec();
}
