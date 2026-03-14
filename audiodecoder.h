#pragma once

#include <QAudioFormat>
#include <QByteArray>
#include <QString>

class QAudioDevice;

struct DecodedAudio final {
    QAudioFormat format;
    QByteArray pcm;
    qint64 durationMs = 0;
};

bool decodeAudioFileForDevice(const QString &filePath,
                              const QAudioDevice &device,
                              DecodedAudio *outAudio,
                              QString *outErrorText);

