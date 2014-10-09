#-------------------------------------------------
#
# Project created by QtCreator 2014-09-17T10:24:01
#
#-------------------------------------------------

QT       += core gui multimedia winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myMusicPlayer
TEMPLATE = app


SOURCES += main.cpp\
    about.cpp \
    login/login.cpp \
    volumebutton.cpp \
    mymusicplayer.cpp

HEADERS  += \
    about.h \
    login/login.h \
    volumebutton.h \
    mymusicplayer.h

FORMS    += \
    about.ui \
    login/login.ui \
    mymusicplayer.ui

OTHER_FILES += \
    resources/lrc/Selena Gomez-Love You Like a Love Song.lrc

RESOURCES += \
    musicplayer.qrc
