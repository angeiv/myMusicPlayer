#include "playerengine.h"

#include <QAudioOutput>
#include <QRandomGenerator>

PlayerEngine::PlayerEngine(QObject *parent)
    : QObject(parent)
    , m_playlist(new PlaylistModel(this))
    , m_lyrics(new LyricsModel(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player.setAudioOutput(m_audioOutput);
    setVolume(80);

    connect(&m_player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        m_lyrics->setPosition(position);
        emit positionChanged();
    });

    connect(&m_player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        if (m_duration == duration) {
            return;
        }
        m_duration = duration;
        emit durationChanged();
    });

    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, [this] {
        emit playingChanged();
    });

    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            playNextInternal();
        }
    });

    connect(&m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &errorString) {
        m_lastError = errorString;
        emit errorMessageChanged();
    });

    if (m_playlist->rowCount() > 0) {
        setCurrentIndex(0);
    }
}

PlaylistModel *PlayerEngine::playlist() const
{
    return m_playlist;
}

LyricsModel *PlayerEngine::lyrics() const
{
    return m_lyrics;
}

int PlayerEngine::currentIndex() const
{
    return m_currentIndex;
}

void PlayerEngine::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_playlist->rowCount()) {
        return;
    }
    if (m_currentIndex == index) {
        return;
    }

    const bool autoPlay = playing();
    m_currentIndex = index;
    emit currentIndexChanged();
    emit currentTrackChanged();

    applyCurrentSource(autoPlay);
}

QString PlayerEngine::currentTitle() const
{
    return m_playlist->trackAt(m_currentIndex).title;
}

QString PlayerEngine::currentArtist() const
{
    return m_playlist->trackAt(m_currentIndex).artist;
}

qint64 PlayerEngine::position() const
{
    return m_player.position();
}

void PlayerEngine::setPosition(qint64 position)
{
    if (qAbs(m_player.position() - position) <= 50) {
        return;
    }
    m_player.setPosition(position);
}

qint64 PlayerEngine::duration() const
{
    return m_duration;
}

bool PlayerEngine::playing() const
{
    return m_player.playbackState() == QMediaPlayer::PlayingState;
}

int PlayerEngine::volume() const
{
    return qRound(m_audioOutput->volume() * 100.0f);
}

void PlayerEngine::setVolume(int volume)
{
    const int v = qBound(0, volume, 100);
    const float normalized = static_cast<float>(v) / 100.0f;
    if (qFuzzyCompare(m_audioOutput->volume(), normalized)) {
        return;
    }
    m_audioOutput->setVolume(normalized);
    emit volumeChanged();
}

bool PlayerEngine::muted() const
{
    return m_audioOutput->isMuted();
}

void PlayerEngine::setMuted(bool muted)
{
    if (m_audioOutput->isMuted() == muted) {
        return;
    }
    m_audioOutput->setMuted(muted);
    emit mutedChanged();
}

PlayerEngine::PlaybackMode PlayerEngine::playbackMode() const
{
    return m_playbackMode;
}

void PlayerEngine::setPlaybackMode(PlayerEngine::PlaybackMode mode)
{
    if (m_playbackMode == mode) {
        return;
    }
    m_playbackMode = mode;
    emit playbackModeChanged();
}

void PlayerEngine::togglePlay()
{
    if (playing()) {
        pause();
    } else {
        play();
    }
}

void PlayerEngine::play()
{
    if (m_playlist->rowCount() == 0) {
        return;
    }
    if (m_currentIndex < 0) {
        setCurrentIndex(0);
    }
    if (m_player.source().isEmpty()) {
        applyCurrentSource(false);
    }
    m_player.play();
}

void PlayerEngine::pause()
{
    m_player.pause();
}

void PlayerEngine::next()
{
    playNextInternal();
}

void PlayerEngine::previous()
{
    playPreviousInternal();
}

void PlayerEngine::addFiles(const QStringList &filePaths)
{
    const bool wasEmpty = m_playlist->rowCount() == 0;
    m_playlist->addFiles(filePaths);
    if (wasEmpty && m_playlist->rowCount() > 0) {
        setCurrentIndex(0);
    }
}

int PlayerEngine::addFilesAndPlay(const QStringList &filePaths)
{
    if (filePaths.isEmpty()) {
        return -1;
    }

    const int start = m_playlist->rowCount();
    addFiles(filePaths);
    if (m_playlist->rowCount() == 0) {
        return -1;
    }

    const int index = qBound(0, start, m_playlist->rowCount() - 1);
    setCurrentIndex(index);
    play();
    return index;
}

void PlayerEngine::removeAt(int index)
{
    if (index < 0 || index >= m_playlist->rowCount()) {
        return;
    }

    const bool removingCurrent = (index == m_currentIndex);
    m_playlist->removeAt(index);

    if (m_playlist->rowCount() == 0) {
        m_player.stop();
        m_player.setSource(QUrl());
        m_lyrics->clear();
        m_currentIndex = -1;
        emit currentIndexChanged();
        emit currentTrackChanged();
        return;
    }

    if (index < m_currentIndex) {
        m_currentIndex--;
        emit currentIndexChanged();
        emit currentTrackChanged();
    } else if (removingCurrent) {
        m_currentIndex = qBound(0, m_currentIndex, m_playlist->rowCount() - 1);
        emit currentIndexChanged();
        emit currentTrackChanged();
        applyCurrentSource(playing());
    }
}

void PlayerEngine::clear()
{
    m_player.stop();
    m_player.setSource(QUrl());
    m_lyrics->clear();
    m_playlist->clear();
    if (m_currentIndex != -1) {
        m_currentIndex = -1;
        emit currentIndexChanged();
        emit currentTrackChanged();
    }
}

void PlayerEngine::applyCurrentSource(bool autoPlay)
{
    const auto url = m_playlist->urlAt(m_currentIndex);
    if (url.isEmpty()) {
        return;
    }
    m_player.setSource(url);
    m_lyrics->loadForTrack(url);
    if (autoPlay) {
        m_player.play();
    }
}

void PlayerEngine::playNextInternal()
{
    if (m_playlist->rowCount() == 0 || m_currentIndex < 0) {
        return;
    }

    switch (m_playbackMode) {
    case PlaybackMode::Sequential:
        if (m_currentIndex < m_playlist->rowCount() - 1) {
            setCurrentIndex(m_currentIndex + 1);
            m_player.play();
        } else {
            m_player.stop();
        }
        break;
    case PlaybackMode::Loop:
        setCurrentIndex((m_currentIndex + 1) % m_playlist->rowCount());
        m_player.play();
        break;
    case PlaybackMode::Random:
        setCurrentIndex(QRandomGenerator::global()->bounded(m_playlist->rowCount()));
        m_player.play();
        break;
    case PlaybackMode::CurrentItemInLoop:
        m_player.setPosition(0);
        m_player.play();
        break;
    }
}

void PlayerEngine::playPreviousInternal()
{
    if (m_playlist->rowCount() == 0 || m_currentIndex < 0) {
        return;
    }

    if (m_currentIndex > 0) {
        setCurrentIndex(m_currentIndex - 1);
    } else if (m_playbackMode == PlaybackMode::Loop) {
        setCurrentIndex(m_playlist->rowCount() - 1);
    } else {
        setCurrentIndex(0);
    }
    m_player.play();
}
