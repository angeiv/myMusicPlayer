#pragma once

#include <QObject>
#include <QVariantList>

class PlayerEngine;

class AppController final : public QObject
{
    Q_OBJECT

public:
    explicit AppController(PlayerEngine *player, QObject *parent = nullptr);

    Q_INVOKABLE void pickAndAddFiles();
    Q_INVOKABLE void addDroppedUrls(const QVariantList &urls);

private:
    PlayerEngine *m_player = nullptr;
};

