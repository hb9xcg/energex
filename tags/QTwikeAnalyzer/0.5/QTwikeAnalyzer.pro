# -------------------------------------------------
# Project created by QtCreator 2009-11-12T21:31:17
# -------------------------------------------------
TARGET = QTwikeAnalyzer
TEMPLATE = app
SOURCES += main.cpp \
    twikeanalyzer.cpp \
    downloaddialog.cpp \
    settingsdialog.cpp \
    twikeport.cpp \
    downloader.cpp \
    ihexconverter.cpp \
    checksum.cpp \
    decoder.cpp \
    rawstorage.cpp \
    batteryplot.cpp \
    aboutdialog.cpp \
    finddialog.cpp \
    fileloader.cpp
HEADERS += twikeanalyzer.h \
    downloaddialog.h \
    settingsdialog.h \
    twikeport.h \
    downloader.h \
    ihexconverter.h \
    checksum.h \
    decoder.h \
    protocol.h \
    rawstorage.h \
    batteryplot.h \
    aboutdialog.h \
    finddialog.h \
    fileloader.h
FORMS += twikeanalyzer.ui \
    downloaddialog.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    finddialog.ui
RESOURCES += images.qrc
INCLUDEPATH += /usr/include/qwt/

# LIBS += -lserial
# CONFIG(debug, debug|release):LIBS += -lqextserialportd
# else:LIBS += -lqextserialport
LIBS += -lqextserialport
LIBS += -lqwt
unix:DEFINES = _TTY_POSIX_
win32:DEFINES = _TTY_WIN_

unix {
	target.path = /usr/bin
	INSTALLS += target
}

