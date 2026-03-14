#include "appcontroller.h"

#include "playerengine.h"

#include <QFileDialog>

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

