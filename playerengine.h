#pragma once

#include "lyricsmodel.h"
#include "playlistmodel.h"

#include <QMediaPlayer>
#include <QObject>

class QAudioOutput;

class PlayerEngine final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(PlaylistModel* playlist READ playlist CONSTANT)
    Q_PROPERTY(LyricsModel* lyrics READ lyrics CONSTANT)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY currentTrackChanged)

    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)

    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)

public:
    enum class PlaybackMode {
        Sequential = 0,
        Loop = 1,
        Random = 2,
        CurrentItemInLoop = 3,
    };
    Q_ENUM(PlaybackMode)

    Q_PROPERTY(PlaybackMode playbackMode READ playbackMode WRITE setPlaybackMode NOTIFY playbackModeChanged)

    explicit PlayerEngine(QObject *parent = nullptr);

    PlaylistModel *playlist() const;
    LyricsModel *lyrics() const;

    int currentIndex() const;
    void setCurrentIndex(int index);

    QString currentTitle() const;
    QString currentArtist() const;

    qint64 position() const;
    void setPosition(qint64 position);

    qint64 duration() const;
    bool playing() const;

    int volume() const;
    void setVolume(int volume);

    bool muted() const;
    void setMuted(bool muted);

    PlaybackMode playbackMode() const;
    void setPlaybackMode(PlaybackMode mode);

    Q_INVOKABLE void togglePlay();
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();

    Q_INVOKABLE void addFiles(const QStringList &filePaths);
    Q_INVOKABLE int addFilesAndPlay(const QStringList &filePaths);
    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void clear();

signals:
    void currentIndexChanged();
    void currentTrackChanged();

    void positionChanged();
    void durationChanged();
    void playingChanged();
    void volumeChanged();
    void mutedChanged();
    void playbackModeChanged();

    void errorMessageChanged();

private:
    void applyCurrentSource(bool autoPlay);
    void playNextInternal();
    void playPreviousInternal();

    PlaylistModel *m_playlist = nullptr;
    LyricsModel *m_lyrics = nullptr;
    QMediaPlayer m_player;
    QAudioOutput *m_audioOutput = nullptr;

    int m_currentIndex = -1;
    qint64 m_duration = 0;
    PlaybackMode m_playbackMode = PlaybackMode::Sequential;

    QString m_lastError;
};
