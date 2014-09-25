#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"
#include "about.h"
#include <QtWidgets>
#include <QDebug>
#include <string>
#include <QTextCodec>

myMusicPlayer::myMusicPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::myMusicPlayer)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
    ui->setupUi(this);

    initWindow();

    //连接信号
    connect(actionNew,SIGNAL(triggered()),this,SLOT(openFile()));//打开音乐文件

    connect(actionAbout,SIGNAL(triggered()),this,SLOT(aboutWindow()));//打开关于窗口

    connect(addSong,SIGNAL(clicked()),this,SLOT(addsong()));
    connect(cutSong,SIGNAL(clicked()),this,SLOT(cutsong()));
    connect(tableList,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(chickToPlay()));//双击打开歌曲


    //载入播放列表
    loadFromFile();
}

myMusicPlayer::~myMusicPlayer()
{
    delete ui;
}

void myMusicPlayer::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("打开音乐文件"),
                                                    "",  tr("MP3音乐文件(*.mp3);;全部文件(*.*)"));

    if(filePath.isEmpty())
        return;

    mediaPlayer.setMedia(QUrl::fromLocalFile(filePath));

    //文件名格式大部分为  歌手 - 歌曲名.mp3，分割内容显示在表格中
    QString info = QUrl::fromLocalFile(filePath).fileName();
    info = info.split(".").first();

    QString author = info.split("-").first();
    QString title = info.split("-").last();

    tableList->insertRow(tableList->rowCount());
    tableList->setItem(tableList->rowCount()-1,
                       0,new QTableWidgetItem(title));
    tableList->setItem(tableList->rowCount()-1,
                       1,new QTableWidgetItem(author));

    playList.addMedia(QUrl::fromLocalFile(filePath));

    mediaPlayer.play();

}

void myMusicPlayer::aboutWindow()
{
    About *ab = new About();
    ab->show();
}

void myMusicPlayer::addsong()
{
    QString filePath = QFileDialog::getOpenFileName(this,tr("打开音乐文件"),"",
                                                    tr("MP3音乐文件(*.mp3);;全部文件(*.*)"));
    if(filePath.isEmpty())
        return ;

    QString info = QUrl::fromLocalFile(filePath).fileName();
    info = info.split(".").first();
    QString author = info.split("-").first();
    QString title = info.split("-").last();
    tableList->insertRow(tableList->rowCount());
    tableList->setItem(tableList->rowCount()-1,
                       0,new QTableWidgetItem(title));
    tableList->setItem(tableList->rowCount()-1,
                       1,new QTableWidgetItem(author));
    playList.addMedia(QUrl::fromLocalFile(filePath));

    saveList2File();
}

void myMusicPlayer::cutsong()
{

    playList.removeMedia(tableList->rowCount()-1);

    QTableWidgetItem *item = tableList->currentItem();
        if(item ==Q_NULLPTR)return;
        tableList->removeRow(item->row());

}
void myMusicPlayer::chickToPlay()
{
   //qDebug()<<tableList->currentIndex();
   int rowl = tableList->currentItem()->row();
   //qDebug()<<rowl;
   playList.setCurrentIndex(rowl);
   qDebug()<<playList.currentIndex();
   mediaPlayer.setPlaylist(&playList);
   mediaPlayer.play();
}

void myMusicPlayer::loadFromFile()
{
    playList.load(QUrl::fromLocalFile("plist.m3u"),"m3u");
    int count = playList.mediaCount();
    for(int i = 0; i < count ; i++) {
            QString test = playList.media(i).canonicalUrl().fileName();
            //qDebug()<<test;
            QString info = test;
            info = info.split(".").first();
            QString author = info.split("-").first();
            QString title = info.split("-").last();
            tableList->insertRow(tableList->rowCount());
            tableList->setItem(tableList->rowCount()-1,
                               0,new QTableWidgetItem(title));
            tableList->setItem(tableList->rowCount()-1,
                               1,new QTableWidgetItem(author));
    }
}

void myMusicPlayer::saveList2File()
{
    if(!playList.save(QUrl::fromLocalFile("plist.m3u"),"m3u")) {
        //对话框提示
    }
    //qDebug()<<playList.errorString();
}

void myMusicPlayer::loadFromFile()
{
    playList.load(QUrl::fromLocalFile("plist.m3u"),"m3u");
    int count = playList.mediaCount();
    for(int i = 0; i < count ; i++) {
            QString test = playList.media(i).canonicalUrl().fileName();
            //qDebug()<<test;
            QString info = test;
            info = info.split(".").first();
            QString author = info.split("-").first();
            QString title = info.split("-").last();
            tableList->insertRow(tableList->rowCount());
            tableList->setItem(tableList->rowCount()-1,
                               0,new QTableWidgetItem(title));
            tableList->setItem(tableList->rowCount()-1,
                               1,new QTableWidgetItem(author));
    }
}

void myMusicPlayer::saveList2File()
{
    if(!playList.save(QUrl::fromLocalFile("plist.m3u"),"m3u")) {
        //对话框提示
    }
    //qDebug()<<playList.errorString();
}

void myMusicPlayer::initWindow()
{
    //设置窗口标题、图标和大小
    this->setWindowTitle("音乐魔盒");
    this->setWindowIcon(QIcon(":/resources/img/logo.ico"));
    this->setGeometry(QRect(0,0,780,550));
    this->setMaximumSize(780,550);
    this->setMinimumSize(780,550);

    //新建菜单栏
    menu = new QMenuBar(this);
    menu->setGeometry(QRect(0,0,780,23));
    menuOpen = menu->addMenu(tr("打开(&O)"));
    actionNew = menuOpen->addAction(tr("音乐(&M)..."));
    actionNew->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    menuList = menu->addMenu(tr("列表(&L)"));
    actionList = menuList->addAction(tr("打开本地列表..."));
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
    tableList->setShowGrid(false);
    tableList->setSelectionBehavior(QAbstractItemView::SelectRows);

/*    //表格隔行变色
    QPalette pal;
    pal.setColor(QPalette::Base,QColor(255,0,0));
    pal.setColor(QPalette::AlternateBase,QColor(0,255,0));
    tableList->setPalette(pal);
    tableList->setAlternatingRowColors(true);
*/
    //播放列表滚动条美化
    tableList->horizontalScrollBar()->setStyleSheet("QScrollBar{background:transparent; height:10px;}"
      "QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
      "QScrollBar::handle:hover{background:gray;}"
      "QScrollBar::sub-line{background:transparent;}"
      "QScrollBar::add-line{background:transparent;}");
     tableList->verticalScrollBar()->setStyleSheet("QScrollBar{background:transparent; width: 10px;}"
      "QScrollBar::handle{background:lightgray; border:2px solid transparent; border-radius:5px;}"
      "QScrollBar::handle:hover{background:gray;}"
      "QScrollBar::sub-line{background:transparent;}"
      "QScrollBar::add-line{background:transparent;}");

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

    addSong = new QPushButton(this);
    addSong->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    addSong->setGeometry(521,440,32,32);
    cutSong = new QPushButton(this);
    cutSong->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));
    cutSong->setGeometry(555,440,32,32);

}
