#include "playlistmodel.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

namespace {

QString trackTitleFromBaseName(const QString &baseName)
{
    const auto parts = baseName.split('-', Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        return parts.first().trimmed();
    }
    return baseName.trimmed();
}

QString trackArtistFromBaseName(const QString &baseName)
{
    const auto parts = baseName.split('-', Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        return parts.last().trimmed();
    }
    return QObject::tr("未知歌手");
}

} // namespace

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_tracks.size();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size()) {
        return {};
    }

    const auto &track = m_tracks.at(index.row());
    switch (role) {
    case TitleRole:
        return track.title;
    case ArtistRole:
        return track.artist;
    case UrlRole:
        return track.url;
    case FilePathRole:
        return track.url.isLocalFile() ? track.url.toLocalFile() : track.url.toString();
    default:
        return {};
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    return {
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {UrlRole, "url"},
        {FilePathRole, "filePath"},
    };
}

void PlaylistModel::addFiles(const QStringList &filePaths)
{
    if (filePaths.isEmpty()) {
        return;
    }

    const int start = m_tracks.size();
    beginInsertRows(QModelIndex(), start, start + filePaths.size() - 1);
    for (const auto &path : filePaths) {
        const auto url = QUrl::fromLocalFile(path);
        m_tracks.push_back(makeTrack(url));
    }
    endInsertRows();

    save();
}

void PlaylistModel::removeAt(int index)
{
    if (index < 0 || index >= m_tracks.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_tracks.removeAt(index);
    endRemoveRows();

    save();
}

void PlaylistModel::clear()
{
    if (m_tracks.isEmpty()) {
        return;
    }

    beginResetModel();
    m_tracks.clear();
    endResetModel();

    save();
}

Track PlaylistModel::trackAt(int index) const
{
    if (index < 0 || index >= m_tracks.size()) {
        return {};
    }
    return m_tracks.at(index);
}

QUrl PlaylistModel::urlAt(int index) const
{
    if (index < 0 || index >= m_tracks.size()) {
        return {};
    }
    return m_tracks.at(index).url;
}

bool PlaylistModel::load()
{
    QFile file(storagePath());
    if (!file.exists()) {
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return false;
    }

    const auto obj = doc.object();
    const auto tracksValue = obj.value("tracks");
    if (!tracksValue.isArray()) {
        return false;
    }

    QStringList filePaths;
    for (const auto &entry : tracksValue.toArray()) {
        if (entry.isString()) {
            filePaths.push_back(entry.toString());
        }
    }

    rebuildFromFilePaths(filePaths);
    return true;
}

bool PlaylistModel::save() const
{
    QJsonArray arr;
    for (const auto &track : m_tracks) {
        if (track.url.isLocalFile()) {
            arr.append(track.url.toLocalFile());
        } else {
            arr.append(track.url.toString());
        }
    }

    QJsonObject obj;
    obj.insert("version", 1);
    obj.insert("tracks", arr);

    const auto path = storagePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return file.commit();
}

QString PlaylistModel::storagePath() const
{
    const auto baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(baseDir).filePath("playlist.json");
}

Track PlaylistModel::makeTrack(const QUrl &url) const
{
    Track t;
    t.url = url;

    const QString baseName = url.isLocalFile()
        ? QFileInfo(url.toLocalFile()).completeBaseName()
        : url.fileName();

    t.title = trackTitleFromBaseName(baseName);
    t.artist = trackArtistFromBaseName(baseName);
    return t;
}

void PlaylistModel::rebuildFromFilePaths(const QStringList &filePaths)
{
    beginResetModel();
    m_tracks.clear();
    m_tracks.reserve(filePaths.size());
    for (const auto &path : filePaths) {
        const QUrl url = path.startsWith("file:") ? QUrl(path) : QUrl::fromLocalFile(path);
        m_tracks.push_back(makeTrack(url));
    }
    endResetModel();
}
