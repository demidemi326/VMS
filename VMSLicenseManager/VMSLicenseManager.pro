#-------------------------------------------------
#
# Project created by QtCreator 2015-01-29T10:45:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia xml network xmlpatterns

TARGET = VMSLicenseManager_video
TEMPLATE = app

win32:RC_FILE = VMSLicenseManager.rc


SOURCES += main.cpp\
    ../_servicebase/activationdlg.cpp \
    mainwindow.cpp

HEADERS  += ../_servicebase/activationdlg.h \
    mainwindow.h

INCLUDEPATH += ../_servicebase ../include ../_onvif ../_base

DESTDIR = ../_VMS_Product
QMAKE_LIBDIR += ../_VMS_Product


FORMS    += ../_servicebase/activationdlg.ui \
    mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lTrackDLL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lTrackDLL
else:unix: LIBS += -L$$PWD/../lib/ -lTrackDLL

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include
