#include "lrc.h"

#include <QFile>
#include <QTextStream>

namespace {
/// 歌词刷新间隔（毫秒）
constexpr int kRefreshInterval = 40;
} // namespace

Lrc::Lrc(QWidget *parent)
    : QLabel(parent)
    , timer(new QTimer(this))
{
    setObjectName(QStringLiteral("lyricLabel"));
    setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    setWordWrap(true);
    setTextInteractionFlags(Qt::NoTextInteraction);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(timer, &QTimer::timeout, this, &Lrc::showLrc);
}

Lrc::~Lrc() = default;

void Lrc::addLrcFile(const QString &fn)
{
    filename = fn;

    QFile f(filename);
    if (!f.exists() || !f.open(QFile::ReadOnly | QFile::Text)) {
        hasLrc = false;
        data.clear();
        timer->stop();
        setText(tr("未找到歌词文件"));
        return;
    }

    QTextStream in(&f); // Qt 6 默认 UTF-8，兼容带 BOM 的 LRC
    data = in.readAll();
    f.close();

    hasLrc = !data.trimmed().isEmpty();
    if (!hasLrc) {
        timer->stop();
        setText(tr("歌词文件为空"));
    }
}

void Lrc::setDuration(qint64 dura)
{
    duration = qMax<qint64>(0, dura);
}

void Lrc::startLrc()
{
    if (hasLrc && !timer->isActive())
        timer->start(kRefreshInterval);
}

void Lrc::pauseLrc()
{
    timer->stop();
}

QString Lrc::stampFor(qint64 ms, int digits)
{
    const qint64 totalMs = qMax<qint64>(0, ms);
    const qint64 minutes = totalMs / 60000;
    const qint64 seconds = (totalMs / 1000) % 60;

    // 小数秒：digits=2 -> xx，digits=3 -> xxx
    qint64 fraction = totalMs % 1000;
    if (digits == 2)
        fraction /= 10;

    return QStringLiteral("%1:%2.%3")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'))
        .arg(fraction, digits, 10, QLatin1Char('0'));
}

void Lrc::showLrc()
{
    if (!hasLrc)
        return;
    if (duration <= 0)
        return;

    // 兼容 [mm:ss.xx] 与 [mm:ss.xxx] 两种写法
    const QString twoDigits = stampFor(duration, 2);
    const QString threeDigits = stampFor(duration, 3);
    const QString open = QStringLiteral("[");

    qsizetype start = data.indexOf(open + twoDigits);
    qsizetype stampLength = twoDigits.size();
    if (start < 0) {
        start = data.indexOf(open + threeDigits);
        stampLength = threeDigits.size();
    }
    if (start < 0)
        return; // 当前时间没有对应歌词，保持上一句

    qsizetype lineEnd = data.indexOf(QLatin1Char('\n'), start);
    if (lineEnd < 0)
        lineEnd = data.size();

    QString line = data.mid(start + stampLength + 1, lineEnd - start - stampLength - 1);
    const int bracket = line.lastIndexOf(QLatin1Char(']'));
    if (bracket >= 0)
        line = line.mid(bracket + 1);
    line = line.trimmed();

    if (!line.isEmpty())
        setText(line);
}
