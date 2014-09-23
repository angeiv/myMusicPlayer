#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "about.h"
#include "login/login.h"
#include "musicplayer.h"
#include <QtWinExtras>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
/*    MusicPlayer mp;
    connect(&mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(updateState(QMediaPlayer::State)));
    ui->pushButtonPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(ui->pushButtonPlayPause, SIGNAL(clicked()), this, SLOT(togglePlayback()));
    mp.resize(300, 60);
    mp.show();*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_O_triggered()
{
/*    QString filename;
    filename = QFileDialog::getOpenFileName(this,tr("选择音乐"),"",tr("Music (*.mp3 *.wmv *.jpg"));

    if(filename.isEmpty()) {
        return;
    }
    else {
        QImage* img = new QImage;

        if(! (img->load(filename))) {
            QMessageBox::information(this,tr("提示"),tr("打开音乐失败！"));

            delete img;
            return;
        }
        ui->label->setPixmap(QPixmap::fromImage(*img));
    }*/
}

void MainWindow::on_action_About_triggered()
{
    About *ab = new About();
    ab->show();
}


void MainWindow::on_action_Login_triggered()
{
    login *l = new login();
    l->show();
}

void MainWindow::on_action_X_triggered()
{
    this->close();
}


void MainWindow::on_pushButtonPlayPause_clicked()
{
    //有歌曲播放操作...
    //
    //...
}

void MainWindow::updateState(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::PlayingState) {
        ui->pushButtonPlayPause->setToolTip(tr("Pause"));
        ui->pushButtonPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    } else {
        ui->pushButtonPlayPause->setToolTip(tr("Play"));
        ui->pushButtonPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
}

void MainWindow::togglePlayback()
{
    if (mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia)
        ;//openFile();
    else if (mediaPlayer.state() == QMediaPlayer::PlayingState)
        mediaPlayer.pause();
    else
        mediaPlayer.play();
}

