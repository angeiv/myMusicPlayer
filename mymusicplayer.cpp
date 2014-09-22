#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"

myMusicPlayer::myMusicPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::myMusicPlayer)
{
    ui->setupUi(this);

    initWindow();
}

myMusicPlayer::~myMusicPlayer()
{
    delete ui;
}

void myMusicPlayer::initWindow()
{
    //设置床后标题、图标和大小
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
    actionNew = menuAbout->addAction(tr("关于音乐魔盒..."));

    //设置歌词窗口
    textEdit = new QTextEdit(this);
    textEdit->setGeometry(QRect(0,23,521,451));
    textEdit->setCursor(Qt::PointingHandCursor);
    textEdit->setReadOnly(true);
    //stylesheet

    //设置播放列表
    tableList = new QTableWidget(this);
    tableList->setGeometry(QRect(520,23,260,451));

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

    //progressBar = new
}
