#-------------------------------------------------
# 音乐魔盒 myMusicPlayer
#
# 最低版本：Qt 6.12 LTS
# 依赖 API：QMediaPlayer::setAudioOutput / playbackStateChanged /
#           errorOccurred（Qt 6.2），QAudioOutput、QRandomGenerator（Qt 6.0）
#-------------------------------------------------

QT       += core \
    gui \
    multimedia \
    widgets

TARGET = myMusicPlayer
TEMPLATE = app

CONFIG += c++17

# 源码内使用 <lrc/lrc.h>、<theme.h> 等工程根目录相对路径
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

# ---- 版本断言：低于 Qt 6.12 直接报错，避免出现难以定位的编译错误 ----
!equals(QT_MAJOR_VERSION, 6) {
    error("音乐魔盒需要 Qt 6，当前为 Qt $$QT_VERSION。请安装 Qt 6.12 LTS。")
}
!versionAtLeast(QT_VERSION, 6.12.0) {
    error("音乐魔盒最低需要 Qt 6.12 LTS，当前为 $$QT_VERSION。")
}

SOURCES += main.cpp \
    about.cpp \
    icons.cpp \
    login/login.cpp \
    mymusicplayer.cpp \
    lrc/lrc.cpp \
    theme.cpp

HEADERS  += \
    about.h \
    icons.h \
    login/login.h \
    mymusicplayer.h \
    lrc/lrc.h \
    theme.h

FORMS    += \
    about.ui \
    login/login.ui \
    mymusicplayer.ui

RESOURCES += \
    musicplayer.qrc

win32: RC_ICONS = $$PWD/resources/img/logo.ico
