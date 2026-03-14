#include "playerengine.h"

#include "audiodecoder.h"

#include <QAudioDevice>
#include <QAudio>
#include <QAudioSink>
#include <QMediaDevices>
#include <QRandomGenerator>
#include <QMetaObject>
#include <QtMath>

PlayerEngine::PlayerEngine(QObject *parent)
    : QObject(parent)
    , m_playlist(new PlaylistModel(this))
    , m_lyrics(new LyricsModel(this))
{
    m_pcmBuffer.setBuffer(&m_pcm);
    m_positionTimer.setInterval(150);
    connect(&m_positionTimer, &QTimer::timeout, this, [this] { updatePositionTick(false); });

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
    if (m_pcm.isEmpty() || m_format.sampleRate() <= 0) {
        return 0;
    }
    const int bpf = bytesPerFrame();
    if (bpf <= 0) {
        return 0;
    }

    const qint64 bytePos = m_pcmBuffer.isOpen() ? m_pcmBuffer.pos() : 0;
    const qint64 framePos = bytePos / static_cast<qint64>(bpf);
    return (framePos * 1000) / static_cast<qint64>(m_format.sampleRate());
}

void PlayerEngine::setPosition(qint64 position)
{
    if (m_pcm.isEmpty() || m_format.sampleRate() <= 0) {
        return;
    }

    const qint64 pos = qBound<qint64>(0, position, m_duration);
    if (qAbs(this->position() - pos) <= 50) {
        return;
    }

    const int bpf = bytesPerFrame();
    if (bpf <= 0) {
        return;
    }

    const bool wasPlaying = playing();
    if (m_sink) {
        m_sink->stop();
    }

    const qint64 framePos = (pos * static_cast<qint64>(m_format.sampleRate())) / 1000;
    const qint64 bytePos = qBound<qint64>(0, framePos * static_cast<qint64>(bpf), m_pcm.size());

    if (!m_pcmBuffer.isOpen()) {
        m_pcmBuffer.open(QIODevice::ReadOnly);
    }
    m_pcmBuffer.seek(bytePos);
    updatePositionTick(true);

    if (wasPlaying) {
        startOrResume();
    }
}

qint64 PlayerEngine::duration() const
{
    return m_duration;
}

bool PlayerEngine::playing() const
{
    return m_sink && m_sink->state() == QAudio::ActiveState;
}

int PlayerEngine::volume() const
{
    return m_volume;
}

void PlayerEngine::setVolume(int volume)
{
    const int v = qBound(0, volume, 100);
    if (m_volume == v) {
        return;
    }
    m_volume = v;
    applyOutputVolume();
    emit volumeChanged();
}

bool PlayerEngine::muted() const
{
    return m_muted;
}

void PlayerEngine::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }
    m_muted = muted;
    applyOutputVolume();
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
    if (!ensureDecodedForPlayback()) {
        return;
    }

    startOrResume();
}

void PlayerEngine::pause()
{
    if (!m_sink) {
        return;
    }
    m_sink->suspend();
    m_positionTimer.stop();
    updatePositionTick(true);
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
        stopPlayback();
        resetPlaybackData();
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
    stopPlayback();
    resetPlaybackData();
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

    stopPlayback();
    resetPlaybackData();
    m_lyrics->loadForTrack(url);

    if (autoPlay) {
        play();
    } else {
        updatePositionTick(true);
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
            play();
        } else {
            stopPlayback();
        }
        break;
    case PlaybackMode::Loop:
        setCurrentIndex((m_currentIndex + 1) % m_playlist->rowCount());
        play();
        break;
    case PlaybackMode::Random:
        setCurrentIndex(QRandomGenerator::global()->bounded(m_playlist->rowCount()));
        play();
        break;
    case PlaybackMode::CurrentItemInLoop:
        setPosition(0);
        play();
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
    play();
}

bool PlayerEngine::ensureDecodedForPlayback()
{
    if (!m_pcm.isEmpty() && m_sink) {
        return true;
    }

    const auto url = m_playlist->urlAt(m_currentIndex);
    if (!url.isLocalFile()) {
        m_lastError = tr("仅支持本地文件");
        emit errorMessageChanged();
        return false;
    }

    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    DecodedAudio decoded;
    QString err;
    if (!decodeAudioFileForDevice(url.toLocalFile(), device, &decoded, &err)) {
        m_lastError = err.isEmpty() ? tr("解码失败") : err;
        emit errorMessageChanged();
        return false;
    }

    m_format = decoded.format;
    m_pcm = std::move(decoded.pcm);

    if (m_duration != decoded.durationMs) {
        m_duration = decoded.durationMs;
        emit durationChanged();
    }

    m_pcmBuffer.close();
    m_pcmBuffer.open(QIODevice::ReadOnly);
    m_pcmBuffer.seek(0);
    m_lastPositionMs = -1;
    emit positionChanged();

    if (m_sink) {
        m_sink->stop();
        delete m_sink;
        m_sink = nullptr;
    }

    m_sink = new QAudioSink(device, m_format, this);
    applyOutputVolume();

    connect(m_sink, &QAudioSink::stateChanged, this, [this](QAudio::State state) {
        emit playingChanged();

        if (state == QAudio::ActiveState) {
            if (!m_positionTimer.isActive()) {
                m_positionTimer.start();
            }
        } else {
            if (m_positionTimer.isActive()) {
                m_positionTimer.stop();
            }
            updatePositionTick(true);
        }

        if (state == QAudio::IdleState) {
            if (m_pcmBuffer.pos() >= m_pcm.size() && m_pcm.size() > 0) {
                QMetaObject::invokeMethod(this, [this] { playNextInternal(); }, Qt::QueuedConnection);
            }
        }
    });

    return true;
}

void PlayerEngine::stopPlayback()
{
    if (m_sink) {
        m_sink->stop();
    }
    if (m_positionTimer.isActive()) {
        m_positionTimer.stop();
    }
    emit playingChanged();
}

void PlayerEngine::resetPlaybackData()
{
    if (m_sink) {
        m_sink->stop();
        delete m_sink;
        m_sink = nullptr;
    }
    if (m_pcmBuffer.isOpen()) {
        m_pcmBuffer.close();
    }
    m_pcm.clear();
    m_format = {};

    if (m_duration != 0) {
        m_duration = 0;
        emit durationChanged();
    }

    m_lastPositionMs = -1;
    emit positionChanged();
    emit playingChanged();
}

void PlayerEngine::startOrResume()
{
    if (!m_sink) {
        return;
    }
    if (!m_pcmBuffer.isOpen()) {
        m_pcmBuffer.open(QIODevice::ReadOnly);
    }

    if (m_pcmBuffer.pos() >= m_pcm.size() && m_pcm.size() > 0) {
        m_pcmBuffer.seek(0);
    }

    switch (m_sink->state()) {
    case QAudio::ActiveState:
        break;
    case QAudio::SuspendedState:
        m_sink->resume();
        break;
    default:
        m_sink->start(&m_pcmBuffer);
        break;
    }

    if (!m_positionTimer.isActive()) {
        m_positionTimer.start();
    }
    updatePositionTick(true);
}

void PlayerEngine::updatePositionTick(bool forceEmit)
{
    const qint64 pos = position();
    m_lyrics->setPosition(pos);

    if (forceEmit || m_lastPositionMs < 0 || qAbs(pos - m_lastPositionMs) >= 50) {
        m_lastPositionMs = pos;
        emit positionChanged();
    }
}

void PlayerEngine::applyOutputVolume()
{
    if (!m_sink) {
        return;
    }

    if (m_muted) {
        m_sink->setVolume(0.0);
        return;
    }
    m_sink->setVolume(static_cast<qreal>(m_volume) / 100.0);
}

int PlayerEngine::bytesPerSample() const
{
    switch (m_format.sampleFormat()) {
    case QAudioFormat::Int16:
        return 2;
    case QAudioFormat::Float:
        return 4;
    default:
        return 0;
    }
}

int PlayerEngine::bytesPerFrame() const
{
    const int bps = bytesPerSample();
    if (bps <= 0 || m_format.channelCount() <= 0) {
        return 0;
    }
    return bps * m_format.channelCount();
}
