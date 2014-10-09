#-------------------------------------------------
#
# Project created by QtCreator 2014-09-17T10:24:01
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myMusicPlayer
TEMPLATE = app


SOURCES += main.cpp\
    about.cpp \
    login/login.cpp \
    mymusicplayer.cpp \
    lrc.cpp

HEADERS  += \
    about.h \
    login/login.h \
    mymusicplayer.h \
    lrc.h

FORMS    += \
    about.ui \
    login/login.ui \
    mymusicplayer.ui

RESOURCES += \
    musicplayer.qrc
