TEMPLATE = app
TARGET = QTwikeSimulator
QT += core \
    gui
HEADERS += balancer.h \
    cell.h \
    battery.h \
    powersupply.h \
    load.h \
    qtwikesimulator.h
SOURCES += balancer.cpp \
    cell.cpp \
    battery.cpp \
    powersupply.cpp \
    load.cpp \
    main.cpp \
    qtwikesimulator.cpp
FORMS += balancer.ui \
    cell.ui \
    battery.ui \
    powersupply.ui \
    load.ui \
    qtwikesimulator.ui
RESOURCES += 
