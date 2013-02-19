include (../src/qtnetworkpeer.pri)

TEMPLATE = app
TARGET = sender
INCLUDEPATH += .
QT += widgets
CONFIG -= app_bundle

# Input
HEADERS += sender.h
SOURCES += main.cpp sender.cpp
    
