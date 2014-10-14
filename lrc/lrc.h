#ifndef LRC_H
#define LRC_H

#include <QLabel>
#include <QString>
#include <QTimer>

class Lrc:public QLabel
{
    Q_OBJECT
public:
    Lrc(QWidget *parent = 0);
    ~Lrc();
    void addLrcFile(const QString& fn);
    void setDuration(qint64 dura);
    void startLrc();
    void pauseLrc();

private slots:
    void showLrc();

private:
    QString filename;
    QString data;
    QTimer *timer;
    qint64 duration;
    bool hasLrc;
};

#endif // LRC_H
