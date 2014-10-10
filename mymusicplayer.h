﻿#ifndef MYMUSICPLAYER_H
#define MYMUSICPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMenuBar>
#include <QTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <lrc.h>

namespace Ui {
class myMusicPlayer;
}

class myMusicPlayer : public QWidget
{
    Q_OBJECT

public:
    myMusicPlayer(QWidget *parent = 0);
    void initWindow();
    ~myMusicPlayer();

public slots:
    void openFile();
    void aboutWindow();
    void addsong();
    void cutsong();
    void loginWindow();

private:
    Ui::myMusicPlayer *ui;
    QMenuBar *menu;//菜单栏
    QMenu *menuOpen;//打开
    QAction *actionNew;//打开音乐
    QMenu *menuList;//列表
    QAction *actionList;//本地列表
    QMenu *menuLogin;
    QAction *actionLogin;
    QMenu *menuAbout;//关于
    QAction *actionAbout;//关于音乐魔盒
    QLabel *title;//歌曲名
    QLabel *author;//歌手

    QTableWidget *tableList;//歌曲列表
    QPushButton *btnForward;//上一曲
    QPushButton *btnPlayPause;//播放暂停
    QPushButton *btnBackword;//下一曲
    QSlider *progressBar;//歌曲进度条
    QLabel *timeProgress;//歌曲播放进度时间
    QLabel *volumeShow;//音量
    QSlider *volumeControl;//音量控制
    QPushButton *btnVolume;
    QPushButton *btnVolumeControl;
    QPushButton *addSong;//添加歌曲
    QPushButton *cutSong;//删除歌曲
    QPushButton *btnStart;
    QMediaPlayer mediaPlayer;//音乐播放
    QMediaPlaylist playList;//音乐播放列表

    Lrc *lrc;//歌词

    bool playerMuted = 0;//是否无声

private slots:
    void loadFromFile();
    void saveList2File();
    void doubleClickToPlay();
    void playerPause();
    void playerStart();
    void playerBackward();
    void playerForward();
    void initPosition();//初始化进度条
    void resetPosition();//重新设置进度条
    void updatePosition(qint64 position);//进度条和播放位置显示
    void setPosition(int position);

    //四种播放模式：列表循环，随机播放，单曲循环，顺序播放
    void setPlaybackModeLoop();
    void setPlaybackModeRandom();
    void setPlaybackModeCurrentLoop();
    void setPlaybackModeSequential();

    void getLrc(int z);//获取歌词

    void setMuted();

};

#endif // MYMUSICPLAYER_H
