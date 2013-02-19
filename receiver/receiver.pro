include (../src/qtnetworkpeer.pri)

TEMPLATE = app
TARGET = receiver
INCLUDEPATH += .
QT += widgets

# Input
HEADERS += receiver.h
SOURCES += main.cpp receiver.cpp
    
