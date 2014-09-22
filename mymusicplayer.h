#ifndef MYMUSICPLAYER_H
#define MYMUSICPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
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

private:
    Ui::myMusicPlayer *ui;
    QMenuBar *menu;
    QMenu *menuOpen;
    QMenu *menuAbout;
    QAction *actionNew;
    QTextEdit *textEdit;
    QTableWidget *tableList;
    QPushButton *btnForward;
    QPushButton *btnPlayPause;
    QPushButton *btnBackword;
    QSlider *progressBar;
    QLabel *timeProgress;
    QLabel *volumeShow;
    QSlider *volumeControl;
};

#endif // MYMUSICPLAYER_H
