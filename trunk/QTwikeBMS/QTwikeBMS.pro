TEMPLATE = app
TARGET = QTwikeBMS
QT += core \
    gui
HEADERS += battery.h \
    protocol.h \
    os_thread.h \
    ReceiverThread.h \
    types.h \
    uart.h \
    qtwikebms.h
SOURCES += protocol.c \
    battery.c \
    os_thread.cpp \
    ReceiverThread.cpp \
    uart.cpp \
    main.cpp \
    qtwikebms.cpp
FORMS += qtwikebms.ui
RESOURCES += 
LIBS += -lserial
