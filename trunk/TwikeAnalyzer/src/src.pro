SOURCES += twikeanalyzer.cpp \
           main.cpp
HEADERS += twikeanalyzer.h
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 debug
TARGET = ../bin/twikeanalyzer
RESOURCES = application.qrc
CONFIG -= release

