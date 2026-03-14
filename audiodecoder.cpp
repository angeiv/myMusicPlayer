#include "audiodecoder.h"

#include <QAudioDevice>
#include <QFile>
#include <QFileInfo>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include "third_party/dr_libs/dr_flac.h"
#include "third_party/dr_libs/dr_mp3.h"
#include "third_party/dr_libs/dr_wav.h"

namespace {

constexpr float kInt16ToFloat = 1.0f / 32768.0f;

int bytesPerSampleFor(QAudioFormat::SampleFormat sampleFormat)
{
    switch (sampleFormat) {
    case QAudioFormat::Int16:
        return 2;
    case QAudioFormat::Float:
        return 4;
    default:
        return 0;
    }
}

float sampleToFloat(qint16 s)
{
    return static_cast<float>(s) * kInt16ToFloat;
}

qint16 floatToInt16(float v)
{
    v = std::clamp(v, -1.0f, 1.0f);
    const int scaled = qRound(v * 32767.0f);
    return static_cast<qint16>(std::clamp(scaled, -32768, 32767));
}

float mixSampleAt(const qint16 *samples, quint64 frameIndex, int outChannel, int inChannels, int outChannels)
{
    if (!samples || inChannels <= 0) {
        return 0.0f;
    }

    const quint64 base = frameIndex * static_cast<quint64>(inChannels);

    if (inChannels == outChannels) {
        const int ch = qBound(0, outChannel, inChannels - 1);
        return sampleToFloat(samples[base + static_cast<quint64>(ch)]);
    }

    if (inChannels == 1) {
        return sampleToFloat(samples[base]);
    }

    if (outChannels == 1) {
        const float left = sampleToFloat(samples[base + 0]);
        const float right = sampleToFloat(samples[base + 1]);
        return 0.5f * (left + right);
    }

    if (outChannel == 0) {
        return sampleToFloat(samples[base + 0]);
    }
    if (outChannel == 1) {
        return sampleToFloat(samples[base + 1]);
    }
    return sampleToFloat(samples[base + 0]);
}

QByteArray resampleAndConvertFromInt16(const QByteArray &inPcm,
                                      quint64 inFrames,
                                      int inChannels,
                                      int inSampleRate,
                                      const QAudioFormat &outFormat,
                                      quint64 *outFrames)
{
    if (outFrames) {
        *outFrames = 0;
    }
    if (inFrames == 0 || inChannels <= 0 || inSampleRate <= 0) {
        return {};
    }

    const int outSampleRate = outFormat.sampleRate();
    const int outChannels = outFormat.channelCount();
    const int outBytesPerSample = bytesPerSampleFor(outFormat.sampleFormat());
    if (outSampleRate <= 0 || outChannels <= 0 || outBytesPerSample <= 0) {
        return {};
    }

    const auto *inSamples = reinterpret_cast<const qint16 *>(inPcm.constData());
    const quint64 frames = (inFrames * static_cast<quint64>(outSampleRate)) / static_cast<quint64>(inSampleRate);
    const quint64 effectiveFrames = std::max<quint64>(1, frames);

    QByteArray out;
    const quint64 totalSamples = effectiveFrames * static_cast<quint64>(outChannels);
    const quint64 totalBytes = totalSamples * static_cast<quint64>(outBytesPerSample);
    if (totalBytes > static_cast<quint64>(std::numeric_limits<int>::max())) {
        return {};
    }
    out.resize(static_cast<int>(totalBytes));

    const double srcRatio = static_cast<double>(inSampleRate) / static_cast<double>(outSampleRate);
    const quint64 lastFrame = (inFrames > 0) ? (inFrames - 1) : 0;

    if (outFormat.sampleFormat() == QAudioFormat::Float) {
        auto *dst = reinterpret_cast<float *>(out.data());
        for (quint64 i = 0; i < effectiveFrames; ++i) {
            const double src = static_cast<double>(i) * srcRatio;
            const quint64 idx0 = std::min(static_cast<quint64>(src), lastFrame);
            const quint64 idx1 = std::min(idx0 + 1, lastFrame);
            const float frac = static_cast<float>(src - static_cast<double>(idx0));

            for (int ch = 0; ch < outChannels; ++ch) {
                const float s0 = mixSampleAt(inSamples, idx0, ch, inChannels, outChannels);
                const float s1 = mixSampleAt(inSamples, idx1, ch, inChannels, outChannels);
                const float v = s0 + (s1 - s0) * frac;
                dst[i * static_cast<quint64>(outChannels) + static_cast<quint64>(ch)] = v;
            }
        }
    } else {
        auto *dst = reinterpret_cast<qint16 *>(out.data());
        for (quint64 i = 0; i < effectiveFrames; ++i) {
            const double src = static_cast<double>(i) * srcRatio;
            const quint64 idx0 = std::min(static_cast<quint64>(src), lastFrame);
            const quint64 idx1 = std::min(idx0 + 1, lastFrame);
            const float frac = static_cast<float>(src - static_cast<double>(idx0));

            for (int ch = 0; ch < outChannels; ++ch) {
                const float s0 = mixSampleAt(inSamples, idx0, ch, inChannels, outChannels);
                const float s1 = mixSampleAt(inSamples, idx1, ch, inChannels, outChannels);
                const float v = s0 + (s1 - s0) * frac;
                dst[i * static_cast<quint64>(outChannels) + static_cast<quint64>(ch)] = floatToInt16(v);
            }
        }
    }

    if (outFrames) {
        *outFrames = effectiveFrames;
    }
    return out;
}

bool decodeWavS16(const QString &filePath,
                  QByteArray *outPcm,
                  quint64 *outFrames,
                  int *outChannels,
                  int *outSampleRate,
                  QString *outErrorText)
{
    if (outPcm) {
        outPcm->clear();
    }
    if (outFrames) {
        *outFrames = 0;
    }
    if (outChannels) {
        *outChannels = 0;
    }
    if (outSampleRate) {
        *outSampleRate = 0;
    }

    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    drwav_uint64 totalFrames = 0;

    drwav_int16 *samples = nullptr;
#if defined(Q_OS_WIN)
    samples = drwav_open_file_and_read_pcm_frames_s16_w(reinterpret_cast<const wchar_t *>(filePath.utf16()),
                                                        &channels,
                                                        &sampleRate,
                                                        &totalFrames,
                                                        nullptr);
#else
    const QByteArray path = QFile::encodeName(filePath);
    samples = drwav_open_file_and_read_pcm_frames_s16(path.constData(),
                                                      &channels,
                                                      &sampleRate,
                                                      &totalFrames,
                                                      nullptr);
#endif
    if (!samples || totalFrames == 0 || channels == 0 || sampleRate == 0) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("无法解码 WAV：%1").arg(QFileInfo(filePath).fileName());
        }
        if (samples) {
            drwav_free(samples, nullptr);
        }
        return false;
    }

    const quint64 totalSamples = totalFrames * static_cast<quint64>(channels);
    const quint64 totalBytes = totalSamples * sizeof(drwav_int16);
    if (totalBytes > static_cast<quint64>(std::numeric_limits<int>::max())) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("WAV 文件过大：%1").arg(QFileInfo(filePath).fileName());
        }
        drwav_free(samples, nullptr);
        return false;
    }

    QByteArray pcm;
    pcm.resize(static_cast<int>(totalBytes));
    std::memcpy(pcm.data(), samples, static_cast<size_t>(totalBytes));
    drwav_free(samples, nullptr);

    if (outPcm) {
        *outPcm = std::move(pcm);
    }
    if (outFrames) {
        *outFrames = static_cast<quint64>(totalFrames);
    }
    if (outChannels) {
        *outChannels = static_cast<int>(channels);
    }
    if (outSampleRate) {
        *outSampleRate = static_cast<int>(sampleRate);
    }
    return true;
}

bool decodeFlacS16(const QString &filePath,
                   QByteArray *outPcm,
                   quint64 *outFrames,
                   int *outChannels,
                   int *outSampleRate,
                   QString *outErrorText)
{
    if (outPcm) {
        outPcm->clear();
    }
    if (outFrames) {
        *outFrames = 0;
    }
    if (outChannels) {
        *outChannels = 0;
    }
    if (outSampleRate) {
        *outSampleRate = 0;
    }

#if defined(Q_OS_WIN)
    drflac *flac = drflac_open_file_w(reinterpret_cast<const wchar_t *>(filePath.utf16()), nullptr);
#else
    const QByteArray path = QFile::encodeName(filePath);
    drflac *flac = drflac_open_file(path.constData(), nullptr);
#endif

    if (!flac || flac->channels == 0 || flac->sampleRate == 0) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("无法打开 FLAC：%1").arg(QFileInfo(filePath).fileName());
        }
        if (flac) {
            drflac_close(flac);
        }
        return false;
    }

    const unsigned int channels = flac->channels;
    const unsigned int sampleRate = flac->sampleRate;
    drflac_uint64 totalFrames = flac->totalPCMFrameCount;

    QByteArray pcm;
    if (totalFrames > 0) {
        const quint64 totalSamples = static_cast<quint64>(totalFrames) * static_cast<quint64>(channels);
        const quint64 totalBytes = totalSamples * sizeof(drflac_int16);
        if (totalBytes > static_cast<quint64>(std::numeric_limits<int>::max())) {
            if (outErrorText) {
                *outErrorText = QStringLiteral("FLAC 文件过大：%1").arg(QFileInfo(filePath).fileName());
            }
            drflac_close(flac);
            return false;
        }

        pcm.resize(static_cast<int>(totalBytes));
        const drflac_uint64 framesRead = drflac_read_pcm_frames_s16(flac,
                                                                    totalFrames,
                                                                    reinterpret_cast<drflac_int16 *>(pcm.data()));
        pcm.resize(static_cast<int>(framesRead * static_cast<drflac_uint64>(channels) * sizeof(drflac_int16)));
        totalFrames = framesRead;
    } else {
        constexpr drflac_uint64 kChunkFrames = 4096;
        QByteArray chunk;
        chunk.resize(static_cast<int>(kChunkFrames * channels * sizeof(drflac_int16)));
        while (true) {
            const drflac_uint64 framesRead = drflac_read_pcm_frames_s16(
                flac,
                kChunkFrames,
                reinterpret_cast<drflac_int16 *>(chunk.data()));
            if (framesRead == 0) {
                break;
            }
            pcm.append(chunk.constData(),
                       static_cast<int>(framesRead * static_cast<drflac_uint64>(channels) * sizeof(drflac_int16)));
        }
        totalFrames = static_cast<drflac_uint64>(
            pcm.size() / static_cast<int>(channels * sizeof(drflac_int16)));
    }

    drflac_close(flac);

    if (pcm.isEmpty() || totalFrames == 0) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("无法解码 FLAC：%1").arg(QFileInfo(filePath).fileName());
        }
        return false;
    }

    if (outPcm) {
        *outPcm = std::move(pcm);
    }
    if (outFrames) {
        *outFrames = static_cast<quint64>(totalFrames);
    }
    if (outChannels) {
        *outChannels = static_cast<int>(channels);
    }
    if (outSampleRate) {
        *outSampleRate = static_cast<int>(sampleRate);
    }
    return true;
}

bool decodeMp3S16(const QString &filePath,
                  QByteArray *outPcm,
                  quint64 *outFrames,
                  int *outChannels,
                  int *outSampleRate,
                  QString *outErrorText)
{
    if (outPcm) {
        outPcm->clear();
    }
    if (outFrames) {
        *outFrames = 0;
    }
    if (outChannels) {
        *outChannels = 0;
    }
    if (outSampleRate) {
        *outSampleRate = 0;
    }

#if defined(Q_OS_WIN)
    drmp3 mp3 = {};
    if (!drmp3_init_file_w(&mp3, reinterpret_cast<const wchar_t *>(filePath.utf16()), nullptr)) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("无法打开 MP3：%1").arg(QFileInfo(filePath).fileName());
        }
        return false;
    }

    const unsigned int channels = mp3.channels;
    const unsigned int sampleRate = mp3.sampleRate;
    drmp3_uint64 totalFrames = drmp3_get_pcm_frame_count(&mp3);

    QByteArray pcm;
    if (totalFrames > 0) {
        const quint64 totalSamples = static_cast<quint64>(totalFrames) * static_cast<quint64>(channels);
        const quint64 totalBytes = totalSamples * sizeof(drmp3_int16);
        if (totalBytes > static_cast<quint64>(std::numeric_limits<int>::max())) {
            if (outErrorText) {
                *outErrorText = QStringLiteral("MP3 文件过大：%1").arg(QFileInfo(filePath).fileName());
            }
            drmp3_uninit(&mp3);
            return false;
        }

        pcm.resize(static_cast<int>(totalBytes));
        const drmp3_uint64 framesRead =
            drmp3_read_pcm_frames_s16(&mp3,
                                     totalFrames,
                                     reinterpret_cast<drmp3_int16 *>(pcm.data()));
        pcm.resize(static_cast<int>(framesRead * static_cast<drmp3_uint64>(channels) * sizeof(drmp3_int16)));
        totalFrames = framesRead;
    } else {
        constexpr drmp3_uint64 kChunkFrames = 4096;
        QByteArray chunk;
        chunk.resize(static_cast<int>(kChunkFrames * channels * sizeof(drmp3_int16)));
        while (true) {
            const drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(&mp3,
                                                                     kChunkFrames,
                                                                     reinterpret_cast<drmp3_int16 *>(chunk.data()));
            if (framesRead == 0) {
                break;
            }
            pcm.append(chunk.constData(),
                       static_cast<int>(framesRead * static_cast<drmp3_uint64>(channels) * sizeof(drmp3_int16)));
        }
        totalFrames = static_cast<drmp3_uint64>(
            pcm.size() / static_cast<int>(channels * sizeof(drmp3_int16)));
    }

    drmp3_uninit(&mp3);

#else
    drmp3_config config = {};
    drmp3_uint64 totalFrames = 0;
    const QByteArray path = QFile::encodeName(filePath);
    drmp3_int16 *samples =
        drmp3_open_file_and_read_pcm_frames_s16(path.constData(), &config, &totalFrames, nullptr);
    if (!samples || totalFrames == 0 || config.channels == 0 || config.sampleRate == 0) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("无法解码 MP3：%1").arg(QFileInfo(filePath).fileName());
        }
        if (samples) {
            drmp3_free(samples, nullptr);
        }
        return false;
    }

    const quint64 totalSamples = static_cast<quint64>(totalFrames) * static_cast<quint64>(config.channels);
    const quint64 totalBytes = totalSamples * sizeof(drmp3_int16);
    if (totalBytes > static_cast<quint64>(std::numeric_limits<int>::max())) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("MP3 文件过大：%1").arg(QFileInfo(filePath).fileName());
        }
        drmp3_free(samples, nullptr);
        return false;
    }

    QByteArray pcm;
    pcm.resize(static_cast<int>(totalBytes));
    std::memcpy(pcm.data(), samples, static_cast<size_t>(totalBytes));
    drmp3_free(samples, nullptr);

    const unsigned int channels = config.channels;
    const unsigned int sampleRate = config.sampleRate;
#endif

    if (pcm.isEmpty() || totalFrames == 0 || channels == 0 || sampleRate == 0) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("无法解码 MP3：%1").arg(QFileInfo(filePath).fileName());
        }
        return false;
    }

    if (outPcm) {
        *outPcm = std::move(pcm);
    }
    if (outFrames) {
        *outFrames = static_cast<quint64>(totalFrames);
    }
    if (outChannels) {
        *outChannels = static_cast<int>(channels);
    }
    if (outSampleRate) {
        *outSampleRate = static_cast<int>(sampleRate);
    }
    return true;
}

} // namespace

bool decodeAudioFileForDevice(const QString &filePath,
                              const QAudioDevice &device,
                              DecodedAudio *outAudio,
                              QString *outErrorText)
{
    if (!outAudio) {
        return false;
    }

    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("文件不存在：%1").arg(info.fileName());
        }
        return false;
    }

    const QString suffix = info.suffix().toLower();

    QByteArray pcmS16;
    quint64 frames = 0;
    int channels = 0;
    int sampleRate = 0;
    QString decodeError;

    bool ok = false;
    if (suffix == QStringLiteral("wav")) {
        ok = decodeWavS16(info.absoluteFilePath(), &pcmS16, &frames, &channels, &sampleRate, &decodeError);
    } else if (suffix == QStringLiteral("flac")) {
        ok = decodeFlacS16(info.absoluteFilePath(), &pcmS16, &frames, &channels, &sampleRate, &decodeError);
    } else if (suffix == QStringLiteral("mp3")) {
        ok = decodeMp3S16(info.absoluteFilePath(), &pcmS16, &frames, &channels, &sampleRate, &decodeError);
    } else {
        decodeError = QStringLiteral("不支持的格式：%1").arg(info.fileName());
    }

    if (!ok) {
        if (outErrorText) {
            *outErrorText = decodeError;
        }
        return false;
    }

    QAudioFormat inFormat;
    inFormat.setSampleRate(sampleRate);
    inFormat.setChannelCount(channels);
    inFormat.setSampleFormat(QAudioFormat::Int16);

    if (device.isFormatSupported(inFormat)) {
        outAudio->format = inFormat;
        outAudio->pcm = std::move(pcmS16);
        outAudio->durationMs = (frames * 1000) / static_cast<quint64>(sampleRate);
        return true;
    }

    QAudioFormat outFormat = device.preferredFormat();
    if (bytesPerSampleFor(outFormat.sampleFormat()) == 0) {
        QAudioFormat candidate = outFormat;
        candidate.setSampleFormat(QAudioFormat::Int16);
        if (device.isFormatSupported(candidate)) {
            outFormat = candidate;
        } else {
            candidate.setSampleFormat(QAudioFormat::Float);
            if (device.isFormatSupported(candidate)) {
                outFormat = candidate;
            }
        }
    }

    quint64 outFrames = 0;
    const QByteArray outPcm = resampleAndConvertFromInt16(
        pcmS16, frames, channels, sampleRate, outFormat, &outFrames);
    if (outPcm.isEmpty() || outFrames == 0) {
        if (outErrorText) {
            *outErrorText = QStringLiteral("音频格式转换失败：%1").arg(info.fileName());
        }
        return false;
    }

    outAudio->format = outFormat;
    outAudio->pcm = outPcm;
    outAudio->durationMs = (outFrames * 1000) / static_cast<quint64>(outFormat.sampleRate());
    return true;
}
