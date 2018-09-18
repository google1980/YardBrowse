#-------------------------------------------------
#
# Project created by QtCreator 2018-08-14T12:37:02
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YardBrowse
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    painter.cpp \
    sqltablemodel.cpp \
    formcontainerquery.cpp \
    sendtask.cpp \
    global.cpp \
    syncclock.cpp \
    infodialog.cpp

HEADERS += \
        mainwindow.h \
    painter.h \
    datasource.h \
    sqltablemodel.h \
    formcontainerquery.h \
    sendtask.h \
    global.h \
    call_once.h \
    singleton.h \
    syncclock.h \
    infodialog.h

FORMS += \
    formcontainerquery.ui \
    infodialog.ui

RESOURCES += \
    res.qrc

RC_ICONS = earth.ico
