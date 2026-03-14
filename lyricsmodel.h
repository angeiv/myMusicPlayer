#pragma once

#include <QAbstractListModel>
#include <QUrl>

class LyricsModel final : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool hasLyrics READ hasLyrics NOTIFY hasLyricsChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentText READ currentText NOTIFY currentTextChanged)

public:
    enum Role {
        TimeMsRole = Qt::UserRole + 1,
        TextRole,
    };
    Q_ENUM(Role)

    explicit LyricsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool hasLyrics() const;
    int currentIndex() const;
    QString currentText() const;

    Q_INVOKABLE bool loadForTrack(const QUrl &trackUrl);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void setPosition(qint64 positionMs);

signals:
    void hasLyricsChanged();
    void currentIndexChanged();
    void currentTextChanged();

private:
    struct LyricLine {
        int timeMs = 0;
        QString text;
    };

    QString findLyricsFileForTrack(const QUrl &trackUrl) const;
    static QString decodeTextFile(const QString &filePath);
    static QVector<LyricLine> parseLrc(const QString &content);
    void setLines(QVector<LyricLine> lines);
    void setCurrentIndexInternal(int index);

    QVector<LyricLine> m_lines;
    bool m_hasLyrics = false;
    int m_currentIndex = -1;
};

