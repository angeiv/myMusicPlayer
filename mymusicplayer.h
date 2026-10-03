#ifndef MYMUSICPLAYER_H
#define MYMUSICPLAYER_H

#include <QList>
#include <QUrl>
#include <QWidget>

#include <lrc/lrc.h>

class QAction;
class QAudioOutput;
class QCloseEvent;
class QHBoxLayout;
class QLabel;
class QMediaPlayer;
class QMenu;
class QMenuBar;
class QPushButton;
class QSlider;
class QTableWidget;
class QVBoxLayout;

namespace Ui {
class myMusicPlayer;
}

/*!
 * \brief 播放模式。枚举值与 Icons::playbackModeIcon() 的参数一一对应。
 */
enum class PlaybackMode {
    Sequential = 0,       ///< 顺序播放：播完最后一首即停止
    Loop = 1,             ///< 列表循环
    Random = 2,           ///< 随机播放
    CurrentItemInLoop = 3 ///< 单曲循环
};

/*!
 * \brief 主窗口：音乐魔盒。
 *
 * 界面完全由布局管理（不再使用 setGeometry 绝对定位），窗口可自由缩放。
 *
 * 最低 Qt 版本：6.12 LTS（由 myMusicPlayer.pro 的版本断言强制）。
 * 代码用到的较新 API：
 *  - QMediaPlayer::setAudioOutput / playbackStateChanged / errorOccurred：Qt 6.2
 *  - QAudioOutput、QRandomGenerator、QTextStream(UTF-8 默认)：Qt 6.0
 */
class myMusicPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit myMusicPlayer(QWidget *parent = nullptr);
    ~myMusicPlayer() override;

    void initWindow();       ///< 构建全部控件与布局
    void initConnections();  ///< 建立信号槽连接

public slots:
    void openFile();         ///< 打开单个音乐文件并播放
    void addsong();          ///< 批量添加音乐到列表
    void cutsong();          ///< 从列表删除选中歌曲
    void aboutWindow();
    void loginWindow();

private slots:
    void loadFromFile();
    void saveList2File();
    void doubleClickToPlay();
    void playerPause();
    void playerBackward();   ///< 上一曲
    void playerForward();    ///< 下一曲
    void playNext();         ///< 按播放模式自动切歌
    void resetPosition();
    void updatePosition(qint64 position);
    void onSliderPressed();
    void onSliderMoved(int position);
    void onSliderReleased();
    void onDurationChanged(qint64 duration);
    void onMediaError(QMediaPlayer::Error error, const QString &errorString);
    void mediaStateChanged(QMediaPlayer::PlaybackState state);
    void getLrc();
    void setMuted();
    void cyclePlaybackMode();
    void updatePlaybackModeUi();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void buildMenu();
    void buildNowPlayingPanel(QHBoxLayout *parent);
    void buildPlaylistPanel(QHBoxLayout *parent);
    void buildControlBar(QVBoxLayout *parent);

    void playIndex(int index, bool autoplay = true);
    void showIdleState();
    void appendTracks(const QStringList &files, bool autoplayNewest);
    void addPlaylistRow(const QUrl &url);
    void trackInfo(const QUrl &url, QString *title, QString *author) const;
    void refreshPlayButton();
    void updateControlState();
    void updateVolumeUi();

    QString playlistFilePath() const;
    QString formatTime(qint64 ms) const;
    QString playbackModeText() const;

    Ui::myMusicPlayer *ui = nullptr;

    // 菜单
    QMenuBar *menu = nullptr;
    QMenu *menuOpen = nullptr;
    QAction *actionNew = nullptr;
    QMenu *menuList = nullptr;
    QAction *actionList = nullptr;
    QMenu *menuLogin = nullptr;
    QAction *actionLogin = nullptr;
    QMenu *menuAbout = nullptr;
    QAction *actionAbout = nullptr;

    // 正在播放面板
    QLabel *title = nullptr;
    QLabel *author = nullptr;
    Lrc *lrc = nullptr;

    // 播放列表
    QTableWidget *tableList = nullptr;

    // 控制条
    QPushButton *btnMode = nullptr;
    QPushButton *btnPrev = nullptr;
    QPushButton *btnPlayPause = nullptr;
    QPushButton *btnNext = nullptr;
    QPushButton *btnVolume = nullptr;
    QPushButton *addSong = nullptr;
    QPushButton *cutSong = nullptr;
    QSlider *progressBar = nullptr;
    QSlider *volumeControl = nullptr;
    QLabel *timeProgress = nullptr;
    QLabel *totalTime = nullptr;
    QLabel *volumeShow = nullptr;

    // 播放核心
    QMediaPlayer mediaPlayer;
    QAudioOutput *audioOutput = nullptr;
    qint64 totalDuration = 0; ///< 当前音轨总时长（毫秒）
    int sliderRange = 0;      ///< 进度条量程（毫秒，落在 int 范围内）
    bool seeking = false;     ///< 用户正在拖动进度条

    // 播放列表数据
    QList<QUrl> playlist;
    int currentIndex = -1;
    PlaybackMode playbackMode = PlaybackMode::Loop;
};

#endif // MYMUSICPLAYER_H
