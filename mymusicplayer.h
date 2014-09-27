#ifndef MYMUSICPLAYER_H
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
    QTextEdit *textEdit;//歌词区
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



private slots:
    void loadFromFile();
    void saveList2File();
    void doubleClickToPlay();
    void playerPause();
    void playerStart();
    void playerNext();
    void playerForward();
};

#endif // MYMUSICPLAYER_H
