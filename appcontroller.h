#pragma once

#include <QObject>

class PlayerEngine;

class AppController final : public QObject
{
    Q_OBJECT

public:
    explicit AppController(PlayerEngine *player, QObject *parent = nullptr);

    Q_INVOKABLE void pickAndAddFiles();

private:
    PlayerEngine *m_player = nullptr;
};

