#ifndef LRC_H
#define LRC_H

#include <QLabel>
#include <QString>
#include <QTimer>

/*!
 * \brief 歌词显示控件。
 *
 * 播放位置由外部（myMusicPlayer::updatePosition）通过 setDuration() 推送，
 * 本控件只负责按时间戳高亮当前歌词并渲染，避免内部计时与播放器进度不一致。
 * 计时器仅用于降低重绘频率。
 */
class Lrc : public QLabel
{
    Q_OBJECT

public:
    explicit Lrc(QWidget *parent = nullptr);
    ~Lrc() override;

    /// 载入 LRC 文件；文件不存在或为空时进入「无歌词」状态。
    void addLrcFile(const QString &fn);

    /// 设置当前播放位置（毫秒）。
    void setDuration(qint64 dura);

    /// 开始滚动歌词。
    void startLrc();

    /// 暂停滚动歌词。
    void pauseLrc();

private slots:
    void showLrc();

private:
    /// 生成 LRC 时间戳，digits 为小秒位数（2 或 3）。
    static QString stampFor(qint64 ms, int digits);

    QString filename;
    QString data;
    QTimer *timer = nullptr;
    qint64 duration = 0;
    bool hasLrc = false;
};

#endif // LRC_H
