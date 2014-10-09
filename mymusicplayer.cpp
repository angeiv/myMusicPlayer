#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"
#include "about.h"
#include "login/login.h"
#include <QtWidgets>
#include <QDebug>
#include <string>
#include <QTextCodec>

myMusicPlayer::myMusicPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::myMusicPlayer)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    ui->setupUi(this);

    initWindow();

    //连接信号
    connect(actionNew,SIGNAL(triggered()),this,SLOT(openFile()));//打开音乐文件

    connect(actionAbout,SIGNAL(triggered()),this,SLOT(aboutWindow()));//打开关于窗口

    connect(addSong,SIGNAL(clicked()),this,SLOT(addsong()));
    connect(cutSong,SIGNAL(clicked()),this,SLOT(cutsong()));
    connect(tableList,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(doubleClickToPlay()));//双击打开歌曲
    connect(btnPlayPause,SIGNAL(clicked()),this,SLOT(playerPause()));
    connect(btnStart,SIGNAL(clicked())  ,this,SLOT(playerStart()));
    connect(btnForward,SIGNAL(clicked()),this,SLOT(playerBackward()));
    connect(btnBackword,SIGNAL(clicked()),this,SLOT(playerForward()));

    connect(actionLogin,SIGNAL(triggered()),this,SLOT(loginWindow()));

    connect(&mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(updatePosition(qint64)));//播放进度显示

    //载入播放列表
    loadFromFile();
    playList.setPlaybackMode(QMediaPlaylist::Sequential);
}

myMusicPlayer::~myMusicPlayer()
{
    delete ui;
}

void myMusicPlayer::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("打开音乐文件"),
                                                    "",  tr("MP3音乐文件(*.mp3);;WMV音乐文件(*.wmv *.wma *wav);;全部文件(*.*)"));

    if(filePath.isEmpty())
        return;

    mediaPlayer.setMedia(QUrl::fromLocalFile(filePath));

    //文件名格式大部分为  歌手 - 歌曲名.mp3，分割内容显示在表格中
    QString info = QUrl::fromLocalFile(filePath).fileName();
    info = info.split(".").first();

    QString title = info.split("-").first();
    QString author = info.split("-").last();

    tableList->insertRow(tableList->rowCount());
    tableList->setItem(tableList->rowCount()-1,
                       0,new QTableWidgetItem(title));
    tableList->setItem(tableList->rowCount()-1,
                       1,new QTableWidgetItem(author));

    playList.addMedia(QUrl::fromLocalFile(filePath));
    tableList->setCurrentCell(tableList->rowCount(),0);
    saveList2File();
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
                                                    tr("MP3音乐文件(*.mp3);;WMV音乐文件(*.wmv *.wma *wav);;全部文件(*.*)"));
    if(filePath.isEmpty())
        return ;

    QString info = QUrl::fromLocalFile(filePath).fileName();
    info = info.split(".").first();
    QString title = info.split("-").first().toUtf8();
    QString author = info.split("-").last().toUtf8();
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
    playList.removeMedia(item->row());
    tableList->removeRow(item->row());
    saveList2File();

}

void myMusicPlayer::loginWindow()
{
    login *l = new login();
    l->show();

}

void myMusicPlayer::doubleClickToPlay()
{

    int rowl = tableList->currentItem()->row();

    playList.setCurrentIndex(rowl);

    mediaPlayer.play();

    progressBar->setValue(0);
    timeProgress->setText(tr("00:00/00:00"));

    QString str = tableList->item(rowl,0)->text();
    title->setText(str);
    str = tableList->item(rowl,1)->text();
    author->setText(str);

    getLrc(rowl);

}

void myMusicPlayer::playerPause()
{
    mediaPlayer.pause();
    btnPlayPause->hide();
    btnStart->show();

}

void myMusicPlayer::playerStart()
{
    mediaPlayer.play();
    btnStart->hide();
    btnPlayPause->show();
}

void myMusicPlayer::playerBackward()
{
    int row2 =tableList->rowCount();
    int rowl = tableList->currentItem()->row();
    playList.setCurrentIndex(rowl);

    progressBar->setValue(0);
    timeProgress->setText(tr("00:00/00:00"));

    if(rowl<row2-1)
    {
        mediaPlayer.setMedia(playList.media(rowl+1).canonicalUrl());
        mediaPlayer.play();
        tableList->setCurrentCell(rowl+1,0);
    }
    else
    {
        mediaPlayer.setMedia(playList.media(0).canonicalUrl());
        mediaPlayer.play();
        rowl = 0;
        tableList->setCurrentCell(rowl,0);
    }

}

void myMusicPlayer::playerForward()
{

    int row2 = tableList->rowCount();
    int rowl = tableList->currentItem()->row();
    playList.setCurrentIndex(rowl);
    if(rowl>0)
    {
        mediaPlayer.setMedia(playList.media(rowl-1).canonicalUrl());
        mediaPlayer.play();
        tableList->setCurrentCell(rowl-1,0);
    }
    else
    {
        mediaPlayer.setMedia(playList.media(row2-1).canonicalUrl());
        mediaPlayer.play();
        int rowl = row2;
        tableList->setCurrentCell(rowl-1,0);

    }

}

void myMusicPlayer::initPosition()
{
    progressBar = new QSlider(this);
    progressBar->setOrientation(Qt::Horizontal);
    connect(progressBar,SIGNAL(valueChanged(int)),this,SLOT(setPosition(int)));
    progressBar->setGeometry(40,340,250,20);

    timeProgress = new QLabel(tr("00:00/00:00"), this);
    timeProgress->setGeometry(220,355,85,20);
}

void myMusicPlayer::resetPosition()
{
    progressBar->setValue(0);
    timeProgress->setText(tr("00:00/00:00"));
}

void myMusicPlayer::updatePosition(qint64 position)
{
    QTime durationMusic(0, mediaPlayer.duration() / 60000,(mediaPlayer.duration() - mediaPlayer.duration()/60000*60000)/1000+1);

    progressBar->setValue(position*100/mediaPlayer.duration());
    QTime duration(0, position / 60000, qRound((position % 60000) / 1000.0));
    timeProgress->setText(duration.toString(tr("mm:ss")) + "/" + durationMusic.toString(tr("mm:ss")));

}

void myMusicPlayer::setPosition(int position)
{

    /*    if (mediaPlayer.position()*100/mediaPlayer.duration() != position) {
        updatePosition(position);
    }*/
    //qDebug()<<mediaPlayer.position()*100/mediaPlayer.duration()<<position;
}

void myMusicPlayer::setPlaybackModeLoop()
{
    playList.setPlaybackMode(QMediaPlaylist::Loop);
}

void myMusicPlayer::setPlaybackModeRandom()
{
    playList.setPlaybackMode(QMediaPlaylist::Random);
}

void myMusicPlayer::setPlaybackModeCurrentLoop()
{
    playList.setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
}

void myMusicPlayer::setPlaybackModeSequential()
{
    playList.setPlaybackMode(QMediaPlaylist::Sequential);
}

void myMusicPlayer::getLrc(int z)
{
    QString title=this->tableList->item(z,0)->text();
    QString author = this->tableList->item(z,1)->text();
    QString filename = QString(title+"-"+author+".lrc");
    addLrcFile(filename);
    showLrc();
}

void myMusicPlayer::loadFromFile()
{
    QTextCodec *codec=QTextCodec::codecForName("UTF-8");

    playList.load(QUrl::fromLocalFile(":/resources/list/plist.m3u"),"m3u");
    mediaPlayer.setPlaylist(&playList);
    int count = playList.mediaCount();
    for(int i = 0; i < count ; i++) {
        QString info = playList.media(i).canonicalUrl().fileName().toUtf8().data();
        info = info.split(".").first();
        QString title = info.split("-").first();
        QString author = info.split("-").last();

        QByteArray ba = author.toLatin1();
        char* mm = ba.data();
        author = codec->toUnicode(mm);
        ba = title.toLatin1();
        mm = ba.data();
        title = codec->toUnicode(mm);

        tableList->insertRow(tableList->rowCount());
        tableList->setItem(tableList->rowCount()-1,
                           0,new QTableWidgetItem(title));
        tableList->setItem(tableList->rowCount()-1,
                           1,new QTableWidgetItem(author));
    }
}

void myMusicPlayer::saveList2File()
{
    if(!playList.save(QUrl::fromLocalFile(":/resources/list/plist.m3u"),"m3u")) {
        //对话框提示
    }
}

void myMusicPlayer::addLrcFile(const QString &fn)
{
    filename = fn;
    QFile f(filename);
    if(!f.exists())
    {
        lrc->setText("没有发现歌词文件！");
        hasLrc = false;
        //timer->stop();
    }
    else
    {
        if(!f.open(QFile::ReadOnly|QFile::Text))
        {
            return;
        }
        QTextStream out(&f);
        data = out.readAll();
        f.close();
        hasLrc = true;
    }
}

void myMusicPlayer::setDuration(qint64 dura)
{
    duration = dura;
}

void myMusicPlayer::startLrc()
{
    if(hasLrc)
    {
        timer->start(10);
    }
}

void myMusicPlayer::pauseLrc()
{
    if(hasLrc)
    {
        timer->stop();
    }
}

void myMusicPlayer::showLrc()
{qDebug()<<hasLrc;
    if(hasLrc)
    {
        duration +=10;
        QString tStr;
        QTime currentTime(0,(duration/60000)%60,duration/1000%60,duration%1000);
        QString format = "mm:ss.zzz";
        tStr = currentTime.toString(format);
        tStr.chop(1);
        int pa = data.indexOf(tStr);
        if(pa>0)
        {
            QString lrc = data.mid(pa);
            int pb = lrc.indexOf("\n");
            lrc = lrc.left(pb);
            pb = lrc.lastIndexOf("]");
            lrc = lrc.remove(0,pb+1);
            myMusicPlayer::lrc->setText(lrc);
        }
    }
    else
    {
        lrc->setText("没有歌词文件！");
    }
}

void myMusicPlayer::initWindow()
{
    //设置窗口标题、图标和大小
    this->setWindowTitle("音乐魔盒");
    this->setWindowIcon(QIcon(":/resources/img/logo.ico"));
    this->setGeometry(QRect(0,0,555,474));
    this->setMaximumSize(555,474);
    this->setMinimumSize(555,474);

    //新建菜单栏
    menu = new QMenuBar(this);
    menu->setGeometry(QRect(0,0,780,23));
    menuOpen = menu->addMenu(tr("打开(&O)"));
    actionNew = menuOpen->addAction(tr("音乐(&M)..."));
    actionNew->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    menuList = menu->addMenu(tr("列表(&L)"));
    actionList = menuList->addAction(tr("打开本地列表..."));
    menuLogin = menu->addMenu("登陆");
    actionLogin = menuLogin->addAction(tr("登陆账号..."));
    menuAbout = menu->addMenu(tr("帮助(&H)"));
    actionAbout = menuAbout->addAction(tr("关于音乐魔盒..."));

    //设置播放列表
    tableList = new QTableWidget(this);
    tableList->setGeometry(QRect(320,23,260,451));
    tableList->setColumnCount(2);
    tableList->setColumnWidth(0,150);
    tableList->setRowCount(0);
    tableList->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置禁止修改
    tableList->setSelectionMode(QAbstractItemView::SingleSelection);//设置只可选中单个
    tableList->setHorizontalHeaderLabels(QStringList()<<"歌曲名"<<"歌手");

    tableList->setShowGrid(false);
    tableList->setSelectionBehavior(QAbstractItemView::SelectRows);

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
    title = new QLabel(this);
    author = new QLabel(this);
    lrc = new QLabel(this);

    title->setText("欢迎使用音乐魔盒");
    author->setText("........");
    lrc->setText(".........");

    title->setGeometry(70,42,168,32);
    author->setGeometry(70,74,168,32);
    lrc->setGeometry(70,160,168,32);

    title->setAlignment(Qt::AlignCenter);
    author->setAlignment(Qt::AlignCenter);
    lrc->setAlignment(Qt::AlignCenter);

    btnBackword = new QPushButton(this);
    btnPlayPause = new QPushButton(this);
    btnForward = new QPushButton(this);
    btnStart = new QPushButton(this);

    btnBackword->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    btnForward->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    btnStart->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    btnBackword->setGeometry(62,400,32,32);
    btnPlayPause->setGeometry(102,385,64,64);
    btnStart->setGeometry(102,385,64,64);
    btnForward->setGeometry(174,400,32,32);

    btnBackword->setCursor(Qt::PointingHandCursor);
    btnPlayPause->setCursor(Qt::PointingHandCursor);
    btnStart->setCursor(Qt::PointingHandCursor);
    btnForward->setCursor(Qt::PointingHandCursor);

    initPosition();

    btnVolume = new QPushButton(this);
    btnVolume->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    btnVolume->setGeometry(220,400,32,32);

    addSong = new QPushButton(this);
    addSong->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    addSong->setGeometry(320,440,32,32);
    cutSong = new QPushButton(this);
    cutSong->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));
    cutSong->setGeometry(355,440,32,32);

    //设置默认播放模式：列表循环
    setPlaybackModeLoop();

}
