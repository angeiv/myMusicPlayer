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
    appcontroller.cpp \
    lyricsmodel.cpp \
    playlistmodel.cpp \
    playerengine.cpp

HEADERS  += \
    appcontroller.h \
    lyricsmodel.h \
    playlistmodel.h \
    playerengine.h

RESOURCES += \
    musicplayer.qrc
