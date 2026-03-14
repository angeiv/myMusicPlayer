#-------------------------------------------------
#
# Project created by QtCreator 2014-09-17T10:24:01
#
#-------------------------------------------------

QT       += core \
    gui \
    multimedia \
    qml \
    quick \
    quickcontrols2 \
    widgets

TARGET = myMusicPlayer
TEMPLATE = app


SOURCES += main.cpp\
    about.cpp \
    login/login.cpp \
    mymusicplayer.cpp \
    lrc/lrc.cpp \
    appcontroller.cpp \
    lyricsmodel.cpp \
    playlistmodel.cpp \
    playerengine.cpp

HEADERS  += \
    about.h \
    appcontroller.h \
    login/login.h \
    mymusicplayer.h \
    lrc/lrc.h \
    lyricsmodel.h \
    playlistmodel.h \
    playerengine.h

FORMS    += \
    about.ui \
    login/login.ui \
    mymusicplayer.ui

RESOURCES += \
    musicplayer.qrc
