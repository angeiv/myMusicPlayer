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
    void chickToPlay();
private:
    Ui::myMusicPlayer *ui;
    QMenuBar *menu;
    QMenu *menuOpen;
    QMenu *menuAbout;
    QAction *actionNew;
    QAction *actionAbout;
    QTextEdit *textEdit;
    QTableWidget *tableList;
    QPushButton *btnForward;
    QPushButton *btnPlayPause;
    QPushButton *btnBackword;
    QSlider *progressBar;
    QLabel *timeProgress;
    QLabel *volumeShow;
    QSlider *volumeControl;
    QPushButton *btnVolume;
    QPushButton *btnVolumeControl;
    QPushButton *addSong;
    QPushButton *cutSong;
    QMediaPlayer mediaPlayer;
    QMediaPlaylist playList;

/*private slots:
    */
};

#endif // MYMUSICPLAYER_H
