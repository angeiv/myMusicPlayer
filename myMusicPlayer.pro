#-------------------------------------------------
#
# Project created by QtCreator 2014-09-17T10:24:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myMusicPlayer
TEMPLATE = app


SOURCES += main.cpp\
mainwindow.cpp \
    about.cpp \
    login/login.cpp

HEADERS  += mainwindow.h \
    about.h \
    login/login.h

FORMS    += mainwindow.ui \
    about.ui \
    login/login.ui

OTHER_FILES +=

RESOURCES += \
    musicplayer.qrc
