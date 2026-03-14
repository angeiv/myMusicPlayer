#include "lyricsmodel.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

namespace {

int parseFractionToMs(const QString &fraction)
{
    if (fraction.isEmpty()) {
        return 0;
    }
    const QString f = fraction.left(3);
    bool ok = false;
    const int value = f.toInt(&ok);
    if (!ok) {
        return 0;
    }
    if (f.size() == 1) {
        return value * 100;
    }
    if (f.size() == 2) {
        return value * 10;
    }
    return value;
}

bool parseTimestamp(const QString &token, int *outTimeMs)
{
    // token: "mm:ss.xx" or "m:ss" etc
    const int colon = token.indexOf(':');
    if (colon <= 0) {
        return false;
    }

    bool okMin = false;
    const int minutes = token.left(colon).toInt(&okMin);
    if (!okMin) {
        return false;
    }

    const QString rest = token.mid(colon + 1);
    const int dot = rest.indexOf('.');
    bool okSec = false;
    int seconds = 0;
    int ms = 0;

    if (dot >= 0) {
        seconds = rest.left(dot).toInt(&okSec);
        ms = parseFractionToMs(rest.mid(dot + 1));
    } else {
        seconds = rest.toInt(&okSec);
    }

    if (!okSec || seconds < 0 || seconds >= 60 || minutes < 0) {
        return false;
    }

    *outTimeMs = (minutes * 60 + seconds) * 1000 + ms;
    return true;
}

} // namespace

LyricsModel::LyricsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LyricsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_lines.size();
}

QVariant LyricsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_lines.size()) {
        return {};
    }
    const auto &line = m_lines.at(index.row());
    switch (role) {
    case TimeMsRole:
        return line.timeMs;
    case TextRole:
        return line.text;
    default:
        return {};
    }
}

QHash<int, QByteArray> LyricsModel::roleNames() const
{
    return {
        {TimeMsRole, "timeMs"},
        {TextRole, "text"},
    };
}

bool LyricsModel::hasLyrics() const
{
    return m_hasLyrics;
}

int LyricsModel::currentIndex() const
{
    return m_currentIndex;
}

QString LyricsModel::currentText() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_lines.size()) {
        return {};
    }
    return m_lines.at(m_currentIndex).text;
}

bool LyricsModel::loadForTrack(const QUrl &trackUrl)
{
    const auto filePath = findLyricsFileForTrack(trackUrl);
    if (filePath.isEmpty()) {
        clear();
        return false;
    }

    const QString content = decodeTextFile(filePath);
    if (content.isEmpty()) {
        clear();
        return false;
    }

    setLines(parseLrc(content));
    return m_hasLyrics;
}

void LyricsModel::clear()
{
    setLines({});
}

void LyricsModel::setPosition(qint64 positionMs)
{
    if (m_lines.isEmpty()) {
        setCurrentIndexInternal(-1);
        return;
    }

    const int target = qBound(0LL, positionMs, static_cast<qint64>(INT_MAX));

    int lo = 0;
    int hi = m_lines.size() - 1;
    int best = -1;
    while (lo <= hi) {
        const int mid = lo + (hi - lo) / 2;
        const int t = m_lines.at(mid).timeMs;
        if (t <= target) {
            best = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    setCurrentIndexInternal(best);
}

QString LyricsModel::findLyricsFileForTrack(const QUrl &trackUrl) const
{
    if (!trackUrl.isLocalFile()) {
        return {};
    }

    const QFileInfo trackInfo(trackUrl.toLocalFile());
    const auto baseName = trackInfo.completeBaseName();
    if (baseName.isEmpty()) {
        return {};
    }

    const QString sameDir = QDir(trackInfo.absolutePath()).filePath(baseName + ".lrc");
    if (QFileInfo::exists(sameDir)) {
        return sameDir;
    }

    const QString appDir = QDir(QCoreApplication::applicationDirPath()).filePath("lrc/" + baseName + ".lrc");
    if (QFileInfo::exists(appDir)) {
        return appDir;
    }

    const QString cwd = QDir::current().filePath("lrc/" + baseName + ".lrc");
    if (QFileInfo::exists(cwd)) {
        return cwd;
    }

    return {};
}

QString LyricsModel::decodeTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray bytes = file.readAll();
    file.close();
    if (bytes.isEmpty()) {
        return {};
    }

    {
        QStringDecoder decoder(QStringDecoder::Utf8);
        const QString text = decoder.decode(bytes);
        if (!decoder.hasError() && !text.contains(QChar(0xFFFD))) {
            return text;
        }
    }

    {
        QStringDecoder decoder(QStringDecoder::System);
        const QString text = decoder.decode(bytes);
        if (!decoder.hasError()) {
            return text;
        }
    }

    return QString::fromLocal8Bit(bytes);
}

QVector<LyricsModel::LyricLine> LyricsModel::parseLrc(const QString &content)
{
    QVector<LyricLine> lines;

    const auto rawLines = content.split('\n');
    lines.reserve(rawLines.size());

    for (auto rawLine : rawLines) {
        rawLine = rawLine.trimmed();
        if (rawLine.isEmpty()) {
            continue;
        }

        QVector<int> timeTags;
        int pos = 0;
        while (pos < rawLine.size() && rawLine.at(pos) == '[') {
            const int close = rawLine.indexOf(']', pos + 1);
            if (close < 0) {
                break;
            }
            const QString token = rawLine.mid(pos + 1, close - pos - 1).trimmed();

            int timeMs = 0;
            if (parseTimestamp(token, &timeMs)) {
                timeTags.push_back(timeMs);
            }

            pos = close + 1;
        }

        if (timeTags.isEmpty()) {
            continue;
        }

        const QString text = rawLine.mid(pos).trimmed();
        for (const int t : timeTags) {
            lines.push_back({t, text});
        }
    }

    std::stable_sort(lines.begin(), lines.end(), [](const LyricLine &a, const LyricLine &b) {
        return a.timeMs < b.timeMs;
    });

    return lines;
}

void LyricsModel::setLines(QVector<LyricLine> lines)
{
    beginResetModel();
    m_lines = std::move(lines);
    endResetModel();

    const bool hasLyricsNow = !m_lines.isEmpty();
    if (m_hasLyrics != hasLyricsNow) {
        m_hasLyrics = hasLyricsNow;
        emit hasLyricsChanged();
    }

    setCurrentIndexInternal(m_lines.isEmpty() ? -1 : 0);
}

void LyricsModel::setCurrentIndexInternal(int index)
{
    if (m_currentIndex == index) {
        return;
    }
    m_currentIndex = index;
    emit currentIndexChanged();
    emit currentTextChanged();
}
