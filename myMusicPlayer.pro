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
mainwindow.cpp \
    about.cpp \
    login/login.cpp \
    musicplayer.cpp \
    volumebutton.cpp \
    mymusicplayer.cpp \
    lrc.cpp

HEADERS  += mainwindow.h \
    about.h \
    login/login.h \
    musicplayer.h \
    volumebutton.h \
    mymusicplayer.h \
    lrc.h

FORMS    += mainwindow.ui \
    about.ui \
    login/login.ui \
    mymusicplayer.ui

OTHER_FILES +=

RESOURCES += \
    musicplayer.qrc
