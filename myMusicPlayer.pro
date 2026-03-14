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
    audiodecoder.cpp \
    dr_libs_impl.cpp \
    lyricsmodel.cpp \
    playlistmodel.cpp \
    playerengine.cpp

HEADERS  += \
    appcontroller.h \
    audiodecoder.h \
    lyricsmodel.h \
    playlistmodel.h \
    playerengine.h

INCLUDEPATH += third_party/dr_libs

RESOURCES += \
    musicplayer.qrc
