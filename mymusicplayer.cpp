#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"

#include "about.h"
#include "icons.h"
#include "login/login.h"
#include "theme.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSizePolicy>
#include <QSlider>
#include <QStandardPaths>
#include <QStringList>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

#include <limits>

namespace {

constexpr int kDefaultVolume = 80;   ///< 启动音量（0-100）
constexpr int kIconSmall = 20;       ///< 控制条图标尺寸（逻辑像素）
constexpr int kIconPlay = 26;        ///< 主播放按钮图标尺寸

QString audioFileFilter()
{
    return QStringLiteral("音频文件 (*.mp3 *.flac *.wav *.wma *.aac *.ogg *.m4a);;所有文件 (*.*)");
}

} // namespace

myMusicPlayer::myMusicPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myMusicPlayer)
{
    ui->setupUi(this);

    setObjectName(QStringLiteral("myMusicPlayer"));
    setAttribute(Qt::WA_StyledBackground, true); // 让 QSS 渐变底生效
    setWindowTitle(tr("音乐魔盒"));
    QIcon appIcon = QIcon(QStringLiteral(":/resources/img/logo.ico"));
    if (appIcon.isNull()) // 资源缺失时回退到矢量图标
        appIcon = Icons::icon(Icons::Icon::MusicNote, QColor(Theme::Accent), 64);
    setWindowIcon(appIcon);
    setStyleSheet(Theme::styleSheet());
    setMinimumSize(760, 520);
    resize(960, 620);

    initWindow();
    initConnections();

    // 音频输出必须在 QMediaPlayer 之前配置，播放才能走到指定设备
    audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(kDefaultVolume / 100.0f);
    mediaPlayer.setAudioOutput(audioOutput);

    loadFromFile();
    refreshPlayButton();
    updateVolumeUi();
}

myMusicPlayer::~myMusicPlayer()
{
    delete ui;
}

void myMusicPlayer::closeEvent(QCloseEvent *event)
{
    saveList2File();
    QWidget::closeEvent(event);
}

// ---------------------------------------------------------------------------
// 界面构建
// ---------------------------------------------------------------------------

void myMusicPlayer::initWindow()
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 8, 18, 16);
    rootLayout->setSpacing(12);

    buildMenu();
    rootLayout->addWidget(menu);

    QHBoxLayout *middleLayout = new QHBoxLayout();
    middleLayout->setSpacing(12);
    buildNowPlayingPanel(middleLayout);
    buildPlaylistPanel(middleLayout);
    rootLayout->addLayout(middleLayout, 1);

    buildControlBar(rootLayout);

    updatePlaybackModeUi();
    updateControlState();
}

void myMusicPlayer::buildMenu()
{
    menu = new QMenuBar(this);
    menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    const QColor iconColor(Theme::TextDim);

    menuOpen = menu->addMenu(tr("打开(&O)"));
    actionNew = menuOpen->addAction(tr("音乐(&M)..."));
    actionNew->setIcon(Icons::icon(Icons::Icon::Add, iconColor, kIconSmall));
    actionNew->setShortcut(QKeySequence(QStringLiteral("Ctrl+M")));

    menuList = menu->addMenu(tr("列表(&L)"));
    actionList = menuList->addAction(tr("添加本地音乐..."));
    actionList->setIcon(Icons::icon(Icons::Icon::Playlist, iconColor, kIconSmall));
    actionList->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    menuList->addSeparator();
    QAction *actionRemove = menuList->addAction(tr("移除选中歌曲"));
    actionRemove->setIcon(Icons::icon(Icons::Icon::Remove, iconColor, kIconSmall));
    actionRemove->setShortcut(QKeySequence::Delete);

    menuLogin = menu->addMenu(tr("账户(&A)"));
    actionLogin = menuLogin->addAction(tr("登录账号..."));
    actionLogin->setIcon(Icons::icon(Icons::Icon::Login, iconColor, kIconSmall));

    menuAbout = menu->addMenu(tr("帮助(&H)"));
    actionAbout = menuAbout->addAction(tr("关于音乐魔盒..."));
    actionAbout->setIcon(Icons::icon(Icons::Icon::Info, iconColor, kIconSmall));

    // 移除选中歌曲：等价于工具条上的删除按钮
    connect(actionRemove, &QAction::triggered, this, [this] {
        if (cutSong)
            cutSong->click();
    });
}

void myMusicPlayer::buildNowPlayingPanel(QHBoxLayout *parent)
{
    QFrame *panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("glassPanel"));
    panel->setMinimumWidth(340);

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(22, 18, 22, 20);
    panelLayout->setSpacing(6);

    QLabel *sectionLabel = new QLabel(tr("正在播放"), panel);
    sectionLabel->setObjectName(QStringLiteral("panelTitle"));

    title = new QLabel(panel);
    title->setObjectName(QStringLiteral("songTitle"));
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    title->setMinimumHeight(42);
    title->setTextInteractionFlags(Qt::TextSelectableByMouse);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    title->setWordWrap(true);

    author = new QLabel(panel);
    author->setObjectName(QStringLiteral("songAuthor"));
    author->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    author->setTextInteractionFlags(Qt::TextSelectableByMouse);
    author->setWordWrap(true);

    QFrame *divider = new QFrame(panel);
    divider->setObjectName(QStringLiteral("divider"));
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Plain);

    lrc = new Lrc(panel);

    panelLayout->addWidget(sectionLabel);
    panelLayout->addWidget(title);
    panelLayout->addWidget(author);
    panelLayout->addSpacing(8);
    panelLayout->addWidget(divider);
    panelLayout->addWidget(lrc, 1);

    parent->addWidget(panel, 5);
}

void myMusicPlayer::buildPlaylistPanel(QHBoxLayout *parent)
{
    QFrame *panel = new QFrame(this);
    panel->setObjectName(QStringLiteral("glassPanel"));

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(18, 18, 18, 18);
    panelLayout->setSpacing(8);

    QLabel *sectionLabel = new QLabel(tr("播放列表"), panel);
    sectionLabel->setObjectName(QStringLiteral("panelTitle"));

    tableList = new QTableWidget(this);
    tableList->setObjectName(QStringLiteral("playlistView"));
    tableList->setColumnCount(2);
    tableList->setHorizontalHeaderLabels(QStringList{ tr("歌曲名"), tr("歌手") });
    tableList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableList->setSelectionMode(QAbstractItemView::SingleSelection);
    tableList->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableList->setShowGrid(false);
    tableList->setAlternatingRowColors(true);
    tableList->setWordWrap(false);
    tableList->verticalHeader()->setVisible(false);
    tableList->verticalHeader()->setDefaultSectionSize(34);
    tableList->horizontalHeader()->setHighlightSections(false);
    tableList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    tableList->setColumnWidth(1, 130);
    tableList->setContextMenuPolicy(Qt::NoContextMenu);

    panelLayout->addWidget(sectionLabel);
    panelLayout->addWidget(tableList, 1);

    parent->addWidget(panel, 4);
}

void myMusicPlayer::buildControlBar(QVBoxLayout *parent)
{
    QFrame *bar = new QFrame(this);
    bar->setObjectName(QStringLiteral("glassPanel"));

    QVBoxLayout *barLayout = new QVBoxLayout(bar);
    barLayout->setContentsMargins(18, 12, 18, 14);
    barLayout->setSpacing(10);

    // ---- 进度行 ----
    QHBoxLayout *progressRow = new QHBoxLayout();
    progressRow->setSpacing(12);

    timeProgress = new QLabel(tr("00:00"), bar);
    timeProgress->setObjectName(QStringLiteral("timeLabel"));
    timeProgress->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeProgress->setMinimumWidth(54);

    progressBar = new QSlider(Qt::Horizontal, bar);
    progressBar->setObjectName(QStringLiteral("progressSlider"));
    progressBar->setRange(0, 0);
    progressBar->setTracking(true);

    totalTime = new QLabel(tr("00:00"), bar);
    totalTime->setObjectName(QStringLiteral("timeLabel"));
    totalTime->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    totalTime->setMinimumWidth(54);

    progressRow->addWidget(timeProgress);
    progressRow->addWidget(progressBar, 1);
    progressRow->addWidget(totalTime);

    // ---- 控制行 ----
    QHBoxLayout *controlRow = new QHBoxLayout();
    controlRow->setSpacing(10);

    btnMode = new QPushButton(bar);
    btnMode->setObjectName(QStringLiteral("modeButton"));
    btnMode->setFixedSize(42, 34);
    btnMode->setCursor(Qt::PointingHandCursor);
    btnMode->setFocusPolicy(Qt::NoFocus);

    btnPrev = new QPushButton(bar);
    btnPrev->setObjectName(QStringLiteral("glassButton"));
    btnPrev->setIcon(Icons::icon(Icons::Icon::Previous, QColor(Theme::Text), kIconSmall));
    btnPrev->setFixedSize(42, 34);
    btnPrev->setToolTip(tr("上一曲"));
    btnPrev->setCursor(Qt::PointingHandCursor);
    btnPrev->setFocusPolicy(Qt::NoFocus);

    btnPlayPause = new QPushButton(bar);
    btnPlayPause->setObjectName(QStringLiteral("playButton"));
    btnPlayPause->setIcon(Icons::icon(Icons::Icon::Play, QColor(QStringLiteral("#04222B")), kIconPlay));
    btnPlayPause->setFixedSize(62, 62);
    btnPlayPause->setToolTip(tr("播放"));
    btnPlayPause->setCursor(Qt::PointingHandCursor);
    btnPlayPause->setFocusPolicy(Qt::NoFocus);

    btnNext = new QPushButton(bar);
    btnNext->setObjectName(QStringLiteral("glassButton"));
    btnNext->setIcon(Icons::icon(Icons::Icon::Next, QColor(Theme::Text), kIconSmall));
    btnNext->setFixedSize(42, 34);
    btnNext->setToolTip(tr("下一曲"));
    btnNext->setCursor(Qt::PointingHandCursor);
    btnNext->setFocusPolicy(Qt::NoFocus);

    btnVolume = new QPushButton(bar);
    btnVolume->setObjectName(QStringLiteral("glassButton"));
    btnVolume->setFixedSize(42, 34);
    btnVolume->setCursor(Qt::PointingHandCursor);
    btnVolume->setFocusPolicy(Qt::NoFocus);

    volumeControl = new QSlider(Qt::Horizontal, bar);
    volumeControl->setObjectName(QStringLiteral("volumeSlider"));
    volumeControl->setRange(0, 100);
    volumeControl->setValue(kDefaultVolume);
    volumeControl->setFixedWidth(110);
    volumeControl->setToolTip(tr("音量"));

    volumeShow = new QLabel(bar);
    volumeShow->setObjectName(QStringLiteral("volumeLabel"));
    volumeShow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    addSong = new QPushButton(bar);
    addSong->setObjectName(QStringLiteral("glassButton"));
    addSong->setIcon(Icons::icon(Icons::Icon::Add, QColor(Theme::Text), kIconSmall));
    addSong->setFixedSize(42, 34);
    addSong->setToolTip(tr("添加歌曲"));
    addSong->setCursor(Qt::PointingHandCursor);
    addSong->setFocusPolicy(Qt::NoFocus);

    cutSong = new QPushButton(bar);
    cutSong->setObjectName(QStringLiteral("glassButton"));
    cutSong->setIcon(Icons::icon(Icons::Icon::Remove, QColor(Theme::Text), kIconSmall));
    cutSong->setFixedSize(42, 34);
    cutSong->setToolTip(tr("移除选中歌曲"));
    cutSong->setCursor(Qt::PointingHandCursor);
    cutSong->setFocusPolicy(Qt::NoFocus);

    controlRow->addWidget(btnMode);
    controlRow->addStretch(1);
    controlRow->addWidget(btnPrev);
    controlRow->addWidget(btnPlayPause);
    controlRow->addWidget(btnNext);
    controlRow->addStretch(1);
    controlRow->addWidget(btnVolume);
    controlRow->addWidget(volumeControl);
    controlRow->addWidget(volumeShow);
    controlRow->addSpacing(6);
    controlRow->addWidget(addSong);
    controlRow->addWidget(cutSong);

    barLayout->addLayout(progressRow);
    barLayout->addLayout(controlRow);

    parent->addWidget(bar);
}

void myMusicPlayer::initConnections()
{
    // 菜单与工具条
    connect(actionNew, &QAction::triggered, this, &myMusicPlayer::openFile);
    connect(actionList, &QAction::triggered, this, &myMusicPlayer::addsong);
    connect(actionLogin, &QAction::triggered, this, &myMusicPlayer::loginWindow);
    connect(actionAbout, &QAction::triggered, this, &myMusicPlayer::aboutWindow);
    connect(addSong, &QPushButton::clicked, this, &myMusicPlayer::addsong);
    connect(cutSong, &QPushButton::clicked, this, &myMusicPlayer::cutsong);

    // 列表与控制
    connect(tableList, &QTableWidget::doubleClicked, this, &myMusicPlayer::doubleClickToPlay);
    connect(btnPlayPause, &QPushButton::clicked, this, &myMusicPlayer::playerPause);
    connect(btnPrev, &QPushButton::clicked, this, &myMusicPlayer::playerBackward);
    connect(btnNext, &QPushButton::clicked, this, &myMusicPlayer::playerForward);
    connect(btnMode, &QPushButton::clicked, this, &myMusicPlayer::cyclePlaybackMode);
    connect(btnVolume, &QPushButton::clicked, this, &myMusicPlayer::setMuted);

    // 进度条：用 sliderMoved/sliderReleased 取代 valueChanged，避免与播放回调互相触发
    connect(progressBar, &QSlider::sliderPressed, this, &myMusicPlayer::onSliderPressed);
    connect(progressBar, &QSlider::sliderMoved, this, &myMusicPlayer::onSliderMoved);
    connect(progressBar, &QSlider::sliderReleased, this, &myMusicPlayer::onSliderReleased);

    // 音量
    connect(volumeControl, &QSlider::valueChanged, this, [this](int value) {
        if (audioOutput) {
            audioOutput->setVolume(qBound(0, value, 100) / 100.0f);
            // 从 0 上调音量时自动解除静音
            if (value > 0 && audioOutput->isMuted())
                audioOutput->setMuted(false);
        }
        updateVolumeUi();
    });

    // 播放器
    connect(&mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &myMusicPlayer::mediaStateChanged);
    connect(&mediaPlayer, &QMediaPlayer::positionChanged,
            this, &myMusicPlayer::updatePosition);
    connect(&mediaPlayer, &QMediaPlayer::durationChanged,
            this, &myMusicPlayer::onDurationChanged);
    connect(&mediaPlayer, &QMediaPlayer::errorOccurred,
            this, &myMusicPlayer::onMediaError);
    connect(&mediaPlayer, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia)
                    playNext();
            });
}

// ---------------------------------------------------------------------------
// 播放列表
// ---------------------------------------------------------------------------

void myMusicPlayer::openFile()
{
    const QString filePath = QFileDialog::getOpenFileName(this, tr("打开音乐文件"),
                                                           QString(), audioFileFilter());
    if (filePath.isEmpty())
        return;
    appendTracks(QStringList{ filePath }, true);
}

void myMusicPlayer::addsong()
{
    const QStringList files = QFileDialog::getOpenFileNames(this, tr("添加音乐"),
                                                             QString(), audioFileFilter());
    if (files.isEmpty())
        return;
    appendTracks(files, false);
}

void myMusicPlayer::appendTracks(const QStringList &files, bool autoplayNewest)
{
    int added = 0;
    for (const QString &file : files) {
        const QUrl url = QUrl::fromLocalFile(file);
        if (playlist.contains(url))
            continue; // 去重
        playlist.append(url);
        addPlaylistRow(url);
        ++added;
    }

    if (added == 0) {
        QMessageBox::information(this, tr("提示"), tr("所选歌曲已全部存在于播放列表中。"));
        return;
    }

    saveList2File();
    updateControlState();

    if (autoplayNewest)
        playIndex(playlist.size() - 1, true);
}

void myMusicPlayer::addPlaylistRow(const QUrl &url)
{
    QString titleText;
    QString authorText;
    trackInfo(url, &titleText, &authorText);

    const int row = tableList->rowCount();
    tableList->insertRow(row);
    tableList->setItem(row, 0, new QTableWidgetItem(titleText));
    tableList->setItem(row, 1, new QTableWidgetItem(authorText));
}

void myMusicPlayer::trackInfo(const QUrl &url, QString *title, QString *author) const
{
    // 文件名多为「歌手 - 歌曲名.mp3」，沿用原有约定：第一段作歌曲名，末段作歌手
    QString baseName = QFileInfo(url.toLocalFile()).completeBaseName();
    if (baseName.isEmpty())
        baseName = url.fileName();

    if (baseName.contains(QLatin1Char('-'))) {
        const QStringList parts = baseName.split(QLatin1Char('-'));
        *title = parts.first().trimmed();
        *author = parts.last().trimmed();
    } else {
        *title = baseName;
        *author = tr("未知歌手");
    }
}

void myMusicPlayer::cutsong()
{
    QTableWidgetItem *item = tableList->currentItem();
    if (!item) {
        QMessageBox::information(this, tr("提示"), tr("请先在列表中选择要移除的歌曲。"));
        return;
    }

    const int row = item->row();
    if (row < 0 || row >= playlist.size()) {
        qWarning() << "cutsong: 无效行索引" << row << "，列表长度" << playlist.size();
        return;
    }

    const bool removedCurrent = (row == currentIndex);
    const bool wasPlaying = (mediaPlayer.playbackState() == QMediaPlayer::PlayingState);

    playlist.removeAt(row);
    tableList->removeRow(row);

    if (playlist.isEmpty()) {
        currentIndex = -1;
        mediaPlayer.stop();
        mediaPlayer.setSource(QUrl());
        totalDuration = 0;
        sliderRange = 0;
        totalTime->setText(tr("00:00"));
        showIdleState();
    } else if (removedCurrent) {
        // 删除的正是当前曲目：停在同一行（末行则退到上一行），保持原播放状态
        currentIndex = qMin(row, playlist.size() - 1);
        playIndex(currentIndex, wasPlaying);
    } else if (row < currentIndex) {
        --currentIndex;
        tableList->setCurrentCell(currentIndex, 0);
    }

    saveList2File();
    updateControlState();
}

void myMusicPlayer::loadFromFile()
{
    QString path = playlistFilePath();
    if (!QFile::exists(path)) {
        // 兼容旧版本：读取可执行文件同目录下的 playlist.txt
        const QString legacy = QDir(QCoreApplication::applicationDirPath())
                                   .filePath(QStringLiteral("playlist.txt"));
        if (QFile::exists(legacy))
            path = legacy;
    }

    QFile file(path);
    if (!file.exists()) {
        qInfo() << "播放列表文件不存在：" << path;
        showIdleState();
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开播放列表文件：" << file.errorString();
        showIdleState();
        return;
    }

    playlist.clear();
    tableList->setRowCount(0);

    QTextStream in(&file); // Qt 6 中 QTextStream 默认按 UTF-8 读写
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        const QUrl url = QUrl::fromLocalFile(line);
        if (playlist.contains(url))
            continue;
        playlist.append(url);
        addPlaylistRow(url);
    }
    file.close();

    qInfo() << "播放列表加载完成，共" << playlist.size() << "首";

    if (playlist.isEmpty()) {
        showIdleState();
        return;
    }

    currentIndex = 0;
    tableList->setCurrentCell(0, 0);
    title->setText(tableList->item(0, 0)->text());
    author->setText(tableList->item(0, 1)->text());
    lrc->setText(tr("双击列表中的歌曲即可播放"));
    updateControlState();
}

void myMusicPlayer::saveList2File()
{
    const QString path = playlistFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "无法写入播放列表文件：" << file.errorString();
        return;
    }

    QTextStream out(&file);
    for (const QUrl &url : playlist)
        out << url.toLocalFile() << '\n';
    file.close();
}

QString myMusicPlayer::playlistFilePath() const
{
    // 旧版本写在可执行文件目录，装到受保护目录会失败，改用应用数据目录
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty())
        dir = QApplication::applicationDirPath();
    return QDir(dir).filePath(QStringLiteral("playlist.txt"));
}

void myMusicPlayer::showIdleState()
{
    title->setText(tr("欢迎使用音乐魔盒"));
    author->setText(tr("V%1 · Qt %2")
                        .arg(QStringLiteral("1.0"), QStringLiteral(QT_VERSION_STR)));
    lrc->setText(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    timeProgress->setText(tr("00:00"));
    totalTime->setText(tr("00:00"));
    progressBar->setRange(0, 0);
    progressBar->setValue(0);
    updateControlState();
}

// ---------------------------------------------------------------------------
// 播放控制
// ---------------------------------------------------------------------------

void myMusicPlayer::playIndex(int index, bool autoplay)
{
    if (index < 0 || index >= playlist.size()) {
        qWarning() << "playIndex: 越界索引" << index << "，列表长度" << playlist.size();
        return;
    }

    currentIndex = index;
    tableList->setCurrentCell(currentIndex, 0);
    mediaPlayer.setSource(playlist.at(currentIndex));

    resetPosition();
    title->setText(tableList->item(currentIndex, 0)->text());
    author->setText(tableList->item(currentIndex, 1)->text());
    getLrc();

    if (autoplay)
        mediaPlayer.play();

    updateControlState();
}

void myMusicPlayer::doubleClickToPlay()
{
    playIndex(tableList->currentRow(), true);
}

void myMusicPlayer::playerPause()
{
    if (playlist.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("播放列表为空，请先添加歌曲。"));
        return;
    }
    if (currentIndex < 0) {
        playIndex(0, true);
        return;
    }
    if (mediaPlayer.playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer.pause();
        return;
    }
    if (mediaPlayer.source().isEmpty()) {
        playIndex(currentIndex, true);
        return;
    }
    // 顺序播放播完后停在末尾，此时按播放应从头开始
    if (mediaPlayer.playbackState() == QMediaPlayer::StoppedState
        && totalDuration > 0 && mediaPlayer.position() >= totalDuration - 1000) {
        mediaPlayer.setPosition(0);
    }
    mediaPlayer.play();
}

void myMusicPlayer::playerBackward()
{
    if (playlist.isEmpty())
        return;
    // 播过 3 秒以上时，先回到本曲开头（与主流播放器一致）
    if (currentIndex >= 0 && mediaPlayer.position() > 3000) {
        mediaPlayer.setPosition(0);
        lrc->setDuration(0);
        return;
    }
    playIndex(qMax(0, currentIndex - 1), true);
}

void myMusicPlayer::playerForward()
{
    if (playlist.isEmpty())
        return;
    playIndex((qMax(0, currentIndex) + 1) % playlist.size(), true);
}

void myMusicPlayer::playNext()
{
    if (playlist.isEmpty())
        return;

    const int count = playlist.size();
    switch (playbackMode) {
    case PlaybackMode::CurrentItemInLoop:
        playIndex(currentIndex, true);
        break;
    case PlaybackMode::Loop:
        playIndex((qMax(0, currentIndex) + 1) % count, true);
        break;
    case PlaybackMode::Random: {
        if (count == 1) {
            playIndex(0, true);
            break;
        }
        int next = qMax(0, currentIndex);
        while (next == currentIndex)
            next = QRandomGenerator::global()->bounded(count);
        playIndex(next, true);
        break;
    }
    case PlaybackMode::Sequential:
        if (currentIndex + 1 < count) {
            playIndex(currentIndex + 1, true);
        } else {
            // 顺序播放到队尾：停住并复位进度
            mediaPlayer.stop();
            resetPosition();
        }
        break;
    }
}

void myMusicPlayer::resetPosition()
{
    progressBar->setValue(0);
    timeProgress->setText(tr("00:00"));
    lrc->setDuration(0);
}

void myMusicPlayer::updatePosition(qint64 position)
{
    const qint64 pos = qMax<qint64>(0, position);

    if (!seeking && sliderRange > 0)
        progressBar->setValue(int(qBound<qint64>(0, pos, sliderRange)));

    timeProgress->setText(formatTime(pos));
    // 歌词跟随真实播放位置，避免计时误差累积
    lrc->setDuration(pos);
}

void myMusicPlayer::onDurationChanged(qint64 duration)
{
    totalDuration = qMax<qint64>(0, duration);
    // QSlider 量程是 int；超过 ~24 天的音轨才可能溢出，正常音乐不会
    sliderRange = int(qBound<qint64>(0, totalDuration,
                                     qint64(std::numeric_limits<int>::max())));

    if (!seeking) {
        progressBar->setRange(0, sliderRange);
        progressBar->setValue(int(qBound<qint64>(0, mediaPlayer.position(), sliderRange)));
    }
    totalTime->setText(formatTime(totalDuration));
    updateControlState();
}

void myMusicPlayer::onSliderPressed()
{
    seeking = true;
}

void myMusicPlayer::onSliderMoved(int position)
{
    timeProgress->setText(formatTime(position));
    lrc->setDuration(position);
}

void myMusicPlayer::onSliderReleased()
{
    if (!seeking)
        return;
    seeking = false;

    const qint64 target = qBound<qint64>(0, progressBar->value(), sliderRange);
    mediaPlayer.setPosition(target);
    lrc->setDuration(target);
}

void myMusicPlayer::mediaStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        lrc->startLrc();
        break;
    case QMediaPlayer::PausedState:
        lrc->pauseLrc();
        break;
    case QMediaPlayer::StoppedState:
        lrc->pauseLrc();
        lrc->setDuration(0);
        break;
    }
    refreshPlayButton();
}

void myMusicPlayer::onMediaError(QMediaPlayer::Error error, const QString &errorString)
{
    if (error == QMediaPlayer::NoError)
        return;

    qWarning() << "播放出错：" << errorString;
    if (error == QMediaPlayer::InvalidMedia || error == QMediaPlayer::UnsupportedMediaTrack) {
        QMessageBox::warning(this, tr("无法播放"),
                             tr("音乐魔盒无法播放该文件：\n%1").arg(errorString));
    }
}

void myMusicPlayer::refreshPlayButton()
{
    const bool playing = mediaPlayer.playbackState() == QMediaPlayer::PlayingState;
    btnPlayPause->setIcon(Icons::icon(playing ? Icons::Icon::Pause : Icons::Icon::Play,
                                      QColor(QStringLiteral("#04222B")), kIconPlay));
    btnPlayPause->setToolTip(playing ? tr("暂停") : tr("播放"));
}

// ---------------------------------------------------------------------------
// 音量 / 播放模式 / 歌词
// ---------------------------------------------------------------------------

void myMusicPlayer::setMuted()
{
    if (!audioOutput)
        return;

    const bool wantMuted = !audioOutput->isMuted();
    audioOutput->setMuted(wantMuted);
    if (!wantMuted && volumeControl->value() == 0)
        volumeControl->setValue(kDefaultVolume); // 取消静音但音量为 0 时给个默认值

    updateVolumeUi();
}

void myMusicPlayer::updateVolumeUi()
{
    if (!audioOutput || !volumeShow || !btnVolume)
        return;

    const int value = volumeControl->value();
    const bool muted = audioOutput->isMuted();

    volumeShow->setText(muted ? tr("静音") : tr("%1%").arg(value));
    btnVolume->setToolTip(muted ? tr("取消静音") : tr("静音"));

    Icons::Icon id = Icons::Icon::Volume;
    if (muted || value == 0)
        id = Icons::Icon::Muted;
    else if (value < 50)
        id = Icons::Icon::VolumeLow;
    btnVolume->setIcon(Icons::icon(id, QColor(Theme::Text), kIconSmall));
}

void myMusicPlayer::cyclePlaybackMode()
{
    switch (playbackMode) {
    case PlaybackMode::Sequential:
        playbackMode = PlaybackMode::Loop;
        break;
    case PlaybackMode::Loop:
        playbackMode = PlaybackMode::Random;
        break;
    case PlaybackMode::Random:
        playbackMode = PlaybackMode::CurrentItemInLoop;
        break;
    case PlaybackMode::CurrentItemInLoop:
        playbackMode = PlaybackMode::Sequential;
        break;
    }
    updatePlaybackModeUi();
}

void myMusicPlayer::updatePlaybackModeUi()
{
    btnMode->setIcon(Icons::playbackModeIcon(static_cast<int>(playbackMode),
                                             QColor(Theme::Accent), kIconSmall));
    btnMode->setToolTip(tr("播放模式：%1（点击切换）").arg(playbackModeText()));
    btnMode->setStatusTip(btnMode->toolTip());
}

QString myMusicPlayer::playbackModeText() const
{
    switch (playbackMode) {
    case PlaybackMode::Sequential:       return tr("顺序播放");
    case PlaybackMode::Loop:             return tr("列表循环");
    case PlaybackMode::Random:           return tr("随机播放");
    case PlaybackMode::CurrentItemInLoop: return tr("单曲循环");
    }
    return tr("顺序播放");
}

void myMusicPlayer::getLrc()
{
    if (currentIndex < 0 || currentIndex >= playlist.size()) {
        lrc->setText(tr("音乐魔盒"));
        return;
    }

    const QFileInfo audioInfo(playlist.at(currentIndex).toLocalFile());
    const QString baseName = audioInfo.completeBaseName();
    if (baseName.isEmpty()) {
        lrc->setText(tr("未找到歌词文件"));
        return;
    }

    // 优先：音频同目录同名 .lrc；其次：程序工作目录/程序目录下的 lrc/
    const QStringList candidates{
        audioInfo.dir().filePath(baseName + QStringLiteral(".lrc")),
        QDir::current().filePath(QStringLiteral("lrc/") + baseName + QStringLiteral(".lrc")),
        QDir(QCoreApplication::applicationDirPath())
            .filePath(QStringLiteral("lrc/") + baseName + QStringLiteral(".lrc"))
    };

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            lrc->addLrcFile(path);
            return;
        }
    }

    lrc->setText(tr("未找到歌词文件"));
}

void myMusicPlayer::updateControlState()
{
    const bool hasTracks = !playlist.isEmpty();
    btnPlayPause->setEnabled(hasTracks);
    btnPrev->setEnabled(hasTracks);
    btnNext->setEnabled(hasTracks);
    cutSong->setEnabled(hasTracks);
    progressBar->setEnabled(hasTracks && totalDuration > 0);
}

QString myMusicPlayer::formatTime(qint64 ms) const
{
    const qint64 msClamped = qMax<qint64>(0, ms);
    const qint64 totalSeconds = msClamped / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds / 60) % 60;
    const qint64 seconds = totalSeconds % 60;

    const QChar pad = QLatin1Char('0');
    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, pad)
            .arg(minutes, 2, 10, pad)
            .arg(seconds, 2, 10, pad);
    }
    return QStringLiteral("%1:%2").arg(minutes, 2, 10, pad).arg(seconds, 2, 10, pad);
}

// ---------------------------------------------------------------------------
// 辅助窗口
// ---------------------------------------------------------------------------

void myMusicPlayer::aboutWindow()
{
    auto *about = new About(this);
    about->setAttribute(Qt::WA_DeleteOnClose);
    about->show();
}

void myMusicPlayer::loginWindow()
{
    auto *l = new login(this);
    l->setAttribute(Qt::WA_DeleteOnClose);
    l->show();
}
