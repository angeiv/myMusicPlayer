#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"
#include "about.h"
#include <QtWidgets>
#include <QDebug>
#include <string>

myMusicPlayer::myMusicPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::myMusicPlayer)
{
    ui->setupUi(this);

    initWindow();

    //连接信号
    connect(actionNew,SIGNAL(triggered()),this,SLOT(openFile()));//打开音乐文件

    connect(actionAbout,SIGNAL(triggered()),this,SLOT(aboutWindow()));//打开关于窗口
}

myMusicPlayer::~myMusicPlayer()
{
    delete ui;
}

void myMusicPlayer::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("打开音乐文件"), "",  tr("MP3音乐文件(*.mp3);;全部文件(*.*)"));

    mediaPlayer.setMedia(QUrl::fromLocalFile(filePath));

    QString info = QUrl::fromLocalFile(filePath).fileName();
    info = info.split(".").first();
    //qDebug()<<info;
    QString author = info.split("-").first();
    QString title = info.split("-").last();
//qDebug()<<tableList->rowCount();
    tableList->insertRow(tableList->rowCount());
    tableList->setItem(tableList->rowCount()-1,
                       0,new QTableWidgetItem(title));
    tableList->setItem(tableList->rowCount()-1,
                       1,new QTableWidgetItem(author));

    mediaPlayer.play();
}

void myMusicPlayer::aboutWindow()
{
    About *ab = new About();
    ab->show();
}

void myMusicPlayer::initWindow()
{
    //设置窗口标题、图标和大小
    this->setWindowTitle("音乐魔盒");
    this->setWindowIcon(QIcon(":/resources/img/logo.ico"));
    this->setGeometry(QRect(0,0,780,550));

    //新建菜单栏
    menu = new QMenuBar(this);
    menu->setGeometry(QRect(0,0,780,23));
    menuOpen = menu->addMenu(tr("打开(&O)"));
    actionNew = menuOpen->addAction(tr("音乐(&M)..."));
    actionNew->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    menuAbout = menu->addMenu(tr("帮助(&H)"));
    actionAbout = menuAbout->addAction(tr("关于音乐魔盒..."));

    //设置歌词窗口
    textEdit = new QTextEdit(this);
    textEdit->setGeometry(QRect(0,23,521,451));
    //textEdit->setCursor(Qt::PointingHandCursor);
    textEdit->setReadOnly(true);
    //stylesheet

    //设置播放列表
    tableList = new QTableWidget(this);
    tableList->setGeometry(QRect(520,23,260,451));
    tableList->setColumnCount(2);
    tableList->setColumnWidth(0,150);
    tableList->setRowCount(0);
    tableList->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置禁止修改
    tableList->setSelectionMode(QAbstractItemView::SingleSelection);//设置只可选中单个
    tableList->setHorizontalHeaderLabels(QStringList()<<"歌曲名"<<"歌手");
    //tableList->setItem(0,0,new QTableWidgetItem("1"));

    //设置按钮
    btnBackword = new QPushButton(this);
    btnPlayPause = new QPushButton(this);
    btnForward = new QPushButton(this);

    btnBackword->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    btnForward->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));

    btnBackword->setGeometry(40,495,32,32);
    btnPlayPause->setGeometry(78,478,64,64);
    btnForward->setGeometry(150,495,32,32);

    btnBackword->setCursor(Qt::PointingHandCursor);
    btnPlayPause->setCursor(Qt::PointingHandCursor);
    btnForward->setCursor(Qt::PointingHandCursor);

    progressBar = new QSlider(this);
    progressBar->setOrientation(Qt::Horizontal);
    progressBar->setGeometry(210,500,400,20);

    btnVolume = new QPushButton(this);
    btnVolume->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    btnVolume->setGeometry(620,495,32,32);

}
