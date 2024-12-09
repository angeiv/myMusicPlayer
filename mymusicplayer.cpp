#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"
#include "about.h"
#include "login/login.h"
#include <QtWidgets>
#include <QDebug>
#include <QStringConverter>
#include <QRandomGenerator>

myMusicPlayer::myMusicPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::myMusicPlayer)
{
    ui->setupUi(this);

    initWindow();

    //连接信号
    connect(actionNew,SIGNAL(triggered()),this,SLOT(openFile()));//打开音乐文件

    connect(actionAbout,SIGNAL(triggered()),this,SLOT(aboutWindow()));//打开关于窗口

    connect(addSong,SIGNAL(clicked()),this,SLOT(addsong()));
    connect(cutSong,SIGNAL(clicked()),this,SLOT(cutsong()));
    connect(tableList,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(doubleClickToPlay()));//双击打开歌曲
    connect(btnPlayPause,SIGNAL(clicked()),this,SLOT(playerPause()));
    connect(btnForward,SIGNAL(clicked()),this,SLOT(playerBackward()));
    connect(btnBackword,SIGNAL(clicked()),this,SLOT(playerForward()));

    connect(actionLogin,SIGNAL(triggered()),this,SLOT(loginWindow()));

    connect(&mediaPlayer, SIGNAL(positionChanged(qint64)), this,SLOT(positionChanged(qint64)));//播放进度显示

    connect(&mediaPlayer,SIGNAL(durationChanged(qint64)),this,SLOT(durationChanged(qint64)));
    connect(&mediaPlayer,SIGNAL(positionChanged(qint64)),this,SLOT(updatePosition(qint64)));
    connect(btnVolume,SIGNAL(clicked()),this,SLOT(setMuted()));//设置无声
    connect(btnForward, SIGNAL(clicked()), &mediaPlayer, SLOT(play()));
    connect(btnBackword, SIGNAL(clicked()), &mediaPlayer, SLOT(play()));
    connect(tableList, &QTableWidget::doubleClicked, this, &myMusicPlayer::doubleClickToPlay);
    //注：stop会触发mediaStateChanged，从而next()
    connect(&mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &myMusicPlayer::mediaStateChanged);
    //载入播放列表
    loadFromFile();
    currentIndex = 0;
    playbackMode = PlaybackMode::Sequential;
    
    connect(&mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            playNext();
        }
    });

    QAudioOutput *audioOutput = new QAudioOutput(this);
    mediaPlayer.setAudioOutput(audioOutput);
    qDebug() << "音频输出已设置";
}

myMusicPlayer::~myMusicPlayer()
{
    delete ui;
}

void myMusicPlayer::openFile()
{
    play = false;

    QString filePath = QFileDialog::getOpenFileName(this, tr("打开音乐文件"),
                                                    "",  tr("MP3音乐文件(*.mp3);;WMV音乐文件(*.wmv *.wma *wav);;全部文件(*.*)"));

    if(filePath.isEmpty())
        return;

    //文件名格式大部分为  歌手 - 歌曲名.mp3，分割内容显示在表格中
    QString info = QUrl::fromLocalFile(filePath).fileName();
    info = info.split(".").first();

    QString title = info.split("-").first();
    QString author = info.split("-").last();

    int row = tableList->rowCount();

    tableList->insertRow(row);
    tableList->setItem(row,
                       0,new QTableWidgetItem(title));
    tableList->setItem(row,
                       1,new QTableWidgetItem(author));

    playlist.append(QUrl::fromLocalFile(filePath));
    currentIndex = row;
    tableList->setCurrentCell(row,0);
    saveList2File();
    mediaPlayer.play();

    setLrcText(row);

    btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
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
    int row = tableList->rowCount();
    tableList->insertRow(row);
    tableList->setItem(row,
                       0,new QTableWidgetItem(title));
    tableList->setItem(row,
                       1,new QTableWidgetItem(author));
    playlist.append(QUrl::fromLocalFile(filePath));

    saveList2File();
}

void myMusicPlayer::cutsong()
{
    // 获取当前选中的项目
    QTableWidgetItem *item = tableList->currentItem();
    if (!item) {
        qDebug() << "没有选中要删除的歌曲";
        return;
    }

    int row = item->row();
    if (row < 0 || row >= playlist.size()) {
        qDebug() << "无效的行索引：" << row;
        return;
    }

    bool wasPlaying = !play;  // 记录当前是否在播放
    
    // 如果正在播放当前选中的歌曲
    if (row == currentIndex) {
        mediaPlayer.stop();
        mediaPlayer.setSource(QUrl());  // 清除媒体源
        play = true;
        btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        
        // 重置进度条和时间显示
        progressBar->setValue(0);
        timeProgress->setText(tr("00:00 / 00:00"));
    }

    // 从播放列表和表格中删除
    playlist.removeAt(row);
    tableList->removeRow(row);

    // 更新当前索引
    if (playlist.isEmpty()) {
        // 如果删除后播放列表为空
        currentIndex = -1;
        play = true;
        title->setText("欢迎使用音乐魔盒");
        author->setText("V1.0");
        lrc->setText("音乐魔盒");
    } else {
        // 调整当前索引
        if (row <= currentIndex) {
            currentIndex = qMax(0, currentIndex - 1);
        }
        
        // 确保currentIndex在有效范围内
        currentIndex = qBound(0, currentIndex, playlist.size() - 1);
        tableList->setCurrentCell(currentIndex, 0);
        
        // 更新显示信息
        QTableWidgetItem* titleItem = tableList->item(currentIndex, 0);
        QTableWidgetItem* authorItem = tableList->item(currentIndex, 1);
        if (titleItem && authorItem) {
            title->setText(titleItem->text());
            author->setText(authorItem->text());
            
            // 如果之前在播放，且删除的不是当前播放的歌曲，继续播放新的歌曲
            if (wasPlaying && row != currentIndex) {
                mediaPlayer.setSource(playlist[currentIndex]);
                mediaPlayer.play();
                play = false;
                btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
                setLrcText(currentIndex);
            }
        }
    }

    // 保存更新后的播放列表
    saveList2File();
    qDebug() << "歌曲已删除，当前播放列表还有" << playlist.size() << "首歌曲，当前索引：" << currentIndex;
}

void myMusicPlayer::loginWindow()
{
    login *l = new login();
    l->show();

}

void myMusicPlayer::doubleClickToPlay()
{
    mediaPlayer.stop();
    play = false;

    int rowl = tableList->currentItem()->row();
    currentIndex = rowl;
    
    // 设置媒体源并播放
    mediaPlayer.setSource(playlist[currentIndex]);
    mediaPlayer.play();

    progressBar->setValue(0);
    timeProgress->setText(tr("00:00 / 00:00"));

    setLrcText(rowl);

    btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
}

void myMusicPlayer::playerPause()
{
    // 检查播放列表是否为空
    if (playlist.isEmpty()) {
        qDebug() << "播放列表为空";
        return;
    }

    // 确保currentIndex有效
    if (currentIndex < 0 || currentIndex >= playlist.size()) {
        currentIndex = 0;
        qDebug() << "重置当前索引为0";
    }

    play = !play;
    if(!play) {
        // 检查当前是否有设置媒体源
        if (mediaPlayer.source().isEmpty()) {
            qDebug() << "设置新的媒体源：" << playlist[currentIndex].toString();
            mediaPlayer.setSource(playlist[currentIndex]);
        }
        
        mediaPlayer.play();
        btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        
        // 更新界面显示
        QTableWidgetItem* titleItem = tableList->item(currentIndex, 0);
        QTableWidgetItem* authorItem = tableList->item(currentIndex, 1);
        if (titleItem && authorItem) {
            title->setText(titleItem->text());
            author->setText(authorItem->text());
            setLrcText(currentIndex);
        }
    }
    else {
        mediaPlayer.pause();
        btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
    
    qDebug() << "播放状态：" << (play ? "暂停" : "播放") 
             << "，当前索引：" << currentIndex 
             << "，播放列表大小：" << playlist.size();
}

void myMusicPlayer::playerBackward()//下一曲
{
    if (playlist.isEmpty()) return;
    
    mediaPlayer.stop();
    int rowl = currentIndex;
    int row2 = tableList->rowCount();

    progressBar->setValue(0);
    timeProgress->setText(tr("00:00 / 00:00"));

    if(rowl + 1 == row2) {
        tableList->setCurrentCell(0,0);
        currentIndex = 0;
    }
    else {
        tableList->setCurrentCell(rowl + 1,0);
        currentIndex = rowl + 1;
    }
    
    // 设置媒体源并播放
    mediaPlayer.setSource(playlist[currentIndex]);
    mediaPlayer.play();
    play = false;  // 设置为播放状态
    btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    setLrcText(currentIndex);
}

void myMusicPlayer::playerForward()//前一曲
{
    if (playlist.isEmpty()) return;
    
    mediaPlayer.stop();
    int row1 = currentIndex;
    int row2 = tableList->rowCount();
    
    progressBar->setValue(0);
    timeProgress->setText(tr("00:00 / 00:00"));

    if(row1 < 1) {
        tableList->setCurrentCell(row2 - 1,0);
        currentIndex = row2 - 1;
    }
    else {
        tableList->setCurrentCell(row1 - 1,0);
        currentIndex = row1 - 1;
    }
    
    // 设置媒体源并播放
    mediaPlayer.setSource(playlist[currentIndex]);
    mediaPlayer.play();
    play = false;  // 设置为播放状态
    btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    setLrcText(currentIndex);
}

void myMusicPlayer::initPosition()
{
    progressBar = new QSlider(this);
    progressBar->setOrientation(Qt::Horizontal);
    connect(progressBar,SIGNAL(valueChanged(int)),this,SLOT(setPosition(int)));
    progressBar->setGeometry(40,340,250,20);

    timeProgress = new QLabel(tr("00:00 / 00:00"), this);
    timeProgress->setGeometry(210,355,85,20);
}

void myMusicPlayer::resetPosition()
{
    progressBar->setValue(0);
    timeProgress->setText(tr("00:00 / 00:00"));
}
void myMusicPlayer::positionChanged(qint64 position)
{
    progressBar->setValue(position);

}

void myMusicPlayer::durationChanged(qint64 duration)
{
    progressBar->setRange(0,duration);
    totalDuration = duration/1000;
}
void myMusicPlayer::updatePosition(qint64 currentInfo)
{
    QString tStr;

    currentInfo /= 1000;

    if (currentInfo || totalDuration)
    {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);

        QTime totalTime((totalDuration/3600)%60, (totalDuration/60)%60, totalDuration%60, (totalDuration*1000)%1000);
        QString format = "mm:ss";
        if (totalDuration > 3600)
            format = "hh:mm:ss";

        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    if(currentInfo == totalDuration)
    {
        playerBackward();
    }
    timeProgress->setText(tStr);

}


void myMusicPlayer::setPosition(int position)
{
    if (qAbs(mediaPlayer.position() - position) > 99) {
        progressBar->setValue(position);
        mediaPlayer.setPosition(position);
        lrc->setDuration(position);
    }
}

void myMusicPlayer::setPlaybackModeLoop()
{
    playbackMode = PlaybackMode::Loop;
}

void myMusicPlayer::setPlaybackModeRandom()
{
    playbackMode = PlaybackMode::Random;
}

void myMusicPlayer::setPlaybackModeCurrentLoop()
{
    playbackMode = PlaybackMode::CurrentItemInLoop;
}

void myMusicPlayer::setPlaybackModeSequential()
{
    playbackMode = PlaybackMode::Sequential;
}

void myMusicPlayer::getLrc()
{
    QString filename = playlist[currentIndex].fileName();
    filename ="./lrc/" + filename.split(".").first() + ".lrc";
    lrc->addLrcFile(filename);
}

void myMusicPlayer::setMuted()
{
    playerMuted = !playerMuted;
    mediaPlayer.audioOutput()->setMuted(playerMuted);

    if(playerMuted) {
        btnVolume->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
    else {
        btnVolume->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }
}

void myMusicPlayer::mediaStateChanged(QMediaPlayer::PlaybackState state)
{
    getLrc();
    switch(state)
    {
    case QMediaPlayer::StoppedState:
        lrc->setDuration(0);
        break;
    case QMediaPlayer::PlayingState:
        lrc->startLrc();
        break;
    default:
        lrc->pauseLrc();
        break;
    }
}

void myMusicPlayer::setLrcText(int row)
{
    QString str = tableList->item(row,0)->text();
    title->setText(str);
    str = tableList->item(row,1)->text();
    author->setText(str);
    lrc->setText("音乐魔盒");
}

void myMusicPlayer::loadFromFile()
{
    QString path = QApplication::applicationDirPath() + "/playlist.txt";
    QFile file(path);

    if (!file.exists()) {
        qDebug() << "播放列表文件不存在：" << path;
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开播放列表文件：" << file.errorString();
        return;
    }

    playlist.clear();
    tableList->setRowCount(0);  // 清空表格

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;

        QUrl url = QUrl::fromLocalFile(line);
        playlist.append(url);

        // 更新界面显示
        int row = tableList->rowCount();
        tableList->insertRow(row);
        
        // 从文件名中提取歌手和歌曲名信息
        QString fileName = url.fileName();
        fileName = fileName.split(".").first();  // 移除扩展名
        
        QString title, author;
        if (fileName.contains("-")) {
            QStringList parts = fileName.split("-");
            title = parts.first().trimmed();
            author = parts.last().trimmed();
        } else {
            title = fileName;
            author = tr("未知歌手");
        }
        
        tableList->setItem(row, 0, new QTableWidgetItem(title));
        tableList->setItem(row, 1, new QTableWidgetItem(author));
    }

    file.close();
    qDebug() << "播放列表加载完成，共" << playlist.size() << "项";
    
    // 如果列表不为空，设置当前索引为0
    if (!playlist.isEmpty()) {
        currentIndex = 0;
        tableList->setCurrentCell(0, 0);
    }
}

void myMusicPlayer::saveList2File()
{
    QString path = QApplication::applicationDirPath() + "/playlist.txt";
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开播放列表文件进行写入：" << file.errorString();
        return;
    }

    QTextStream out(&file);
    for (const QUrl &url : playlist) {
        out << url.toLocalFile() << "\n";
    }

    file.close();
    qDebug() << "播放列表已保存，共" << playlist.size() << "项";
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
    actionNew->setShortcut(QKeySequence("Ctrl+M"));
    menuList = menu->addMenu(tr("列表(&L)"));
    actionList = menuList->addAction(tr("打开本地列表..."));
    menuLogin = menu->addMenu("登录");
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
    tableList->setSelectionMode(QAbstractItemView::SingleSelection);//设置只可可选中单个
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
    lrc = new Lrc(this);

    title->setText("欢迎使用音乐魔盒");
    author->setText("V1.0");
    QDate date = QDate::currentDate();
    QString str = date.toString("yyyy-MM-dd");
    lrc->setText(str);

    title->setGeometry(70,42,168,32);
    author->setGeometry(70,74,168,32);
    lrc->setGeometry(50,160,208,32);

    title->setAlignment(Qt::AlignCenter);
    author->setAlignment(Qt::AlignCenter);
    lrc->setAlignment(Qt::AlignCenter);

    btnBackword = new QPushButton(this);
    btnPlayPause = new QPushButton(this);
    btnForward = new QPushButton(this);

    btnBackword->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    btnBackword->setStyleSheet("border:2px groove gray;border-radius:16px;padding:2px 4px;");
    btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    btnPlayPause->setStyleSheet("border:2px groove gray;border-radius:32px;padding:2px 4px;");
    btnForward->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    btnForward->setStyleSheet("border:2px groove gray;border-radius:16px;padding:2px 4px;");

    btnBackword->setGeometry(62,400,32,32);
    btnPlayPause->setGeometry(102,385,64,64);
    btnForward->setGeometry(174,400,32,32);

    btnBackword->setCursor(Qt::PointingHandCursor);
    btnPlayPause->setCursor(Qt::PointingHandCursor);
    btnForward->setCursor(Qt::PointingHandCursor);

    initPosition();

    btnVolume = new QPushButton(this);
    btnVolume->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    btnVolume->setStyleSheet("border:2px groove gray;border-radius:16px;padding:2px 4px;");
    btnVolume->setGeometry(220,400,32,32);

    addSong = new QPushButton(this);
    addSong->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    addSong->setStyleSheet("border:2px groove gray;border-radius:16px;padding:2px 4px;");
    addSong->setGeometry(320,442,32,32);
    cutSong = new QPushButton(this);
    cutSong->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    cutSong->setStyleSheet("border:2px groove gray;border-radius:16px;padding:2px 4px;");
    cutSong->setGeometry(350,442,32,32);

    //设置默认播放模式：列表循环
    setPlaybackModeLoop();

    playerMuted  = false;
    play = true;
}

void myMusicPlayer::playNext()
{
    if (playlist.isEmpty()) return;
    
    switch(playbackMode) {
        case PlaybackMode::Sequential:
            if (currentIndex < playlist.size() - 1) {
                currentIndex++;
                playerBackward();
            } else {
                mediaPlayer.stop();
            }
            break;
            
        case PlaybackMode::Loop:
            playerBackward();  // 已经在playerBackward()中处理了循环逻辑
            break;
            
        case PlaybackMode::Random:
            currentIndex = QRandomGenerator::global()->bounded(playlist.size());
            playerBackward();
            break;
            
        case PlaybackMode::CurrentItemInLoop:
            mediaPlayer.setPosition(0);
            mediaPlayer.play();
            break;
    }
}
