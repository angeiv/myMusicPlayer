#pragma once

#include <QAbstractListModel>
#include <QUrl>

struct Track {
    QUrl url;
    QString title;
    QString artist;
};

class PlaylistModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        UrlRole,
        FilePathRole,
    };
    Q_ENUM(Role)

    explicit PlaylistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addFiles(const QStringList &filePaths);
    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void clear();

    Track trackAt(int index) const;
    QUrl urlAt(int index) const;

    bool load();
    bool save() const;

    QString storagePath() const;

private:
    Track makeTrack(const QUrl &url) const;
    void rebuildFromFilePaths(const QStringList &filePaths);

    QVector<Track> m_tracks;
};

