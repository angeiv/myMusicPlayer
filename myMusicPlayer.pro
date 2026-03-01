#-------------------------------------------------
#
# Project created by QtCreator 2014-09-17T10:24:01
#
#-------------------------------------------------

QT       += core \
    gui \
    multimedia \
    widgets

TARGET = myMusicPlayer
TEMPLATE = app


SOURCES += main.cpp\
    about.cpp \
    login/login.cpp \
    mymusicplayer.cpp \
    lrc/lrc.cpp

HEADERS  += \
    about.h \
    login/login.h \
    mymusicplayer.h \
    lrc/lrc.h

FORMS    += \
    about.ui \
    login/login.ui \
    mymusicplayer.ui

RESOURCES += \
    musicplayer.qrc
