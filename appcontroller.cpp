#include "appcontroller.h"

#include "playerengine.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QSet>
#include <QUrl>

AppController::AppController(PlayerEngine *player, QObject *parent)
    : QObject(parent)
    , m_player(player)
{
}

void AppController::pickAndAddFiles()
{
    if (!m_player) {
        return;
    }

    const QStringList files = QFileDialog::getOpenFileNames(
        nullptr,
        tr("打开音乐文件"),
        QString(),
        tr("音频文件(*.mp3 *.flac *.wav *.wma *.wmv);;全部文件(*.*)"));

    m_player->addFilesAndPlay(files);
}

void AppController::addDroppedUrls(const QVariantList &urls)
{
    if (!m_player || urls.isEmpty()) {
        return;
    }

    const QStringList nameFilters = {"*.mp3", "*.flac", "*.wav", "*.wma", "*.m4a", "*.aac", "*.ogg"};

    QStringList filePaths;
    filePaths.reserve(urls.size());

    QSet<QString> seen;
    for (const auto &entry : urls) {
        const QUrl url = entry.toUrl();
        if (!url.isValid() || !url.isLocalFile()) {
            continue;
        }

        const QString path = url.toLocalFile();
        if (path.isEmpty()) {
            continue;
        }

        const QFileInfo info(path);
        if (!info.exists()) {
            continue;
        }

        if (info.isDir()) {
            QDirIterator it(info.absoluteFilePath(), nameFilters, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                const QString filePath = it.next();
                if (seen.contains(filePath)) {
                    continue;
                }
                seen.insert(filePath);
                filePaths.push_back(filePath);
            }
            continue;
        }

        if (!info.isFile()) {
            continue;
        }

        const QString filePath = info.absoluteFilePath();
        if (seen.contains(filePath)) {
            continue;
        }
        seen.insert(filePath);
        filePaths.push_back(filePath);
    }

    if (filePaths.isEmpty()) {
        return;
    }

    m_player->addFilesAndPlay(filePaths);
}

