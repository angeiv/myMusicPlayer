#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void togglePlayback();
private slots:
    void on_action_O_triggered();

    void on_action_About_triggered();

    void on_action_Login_triggered();

    void on_action_X_triggered();

    void on_pushButtonPlayPause_clicked();

    void updateState(QMediaPlayer::State state);

private:
    Ui::MainWindow *ui;
    QMediaPlayer mediaPlayer;
};

#endif // MAINWINDOW_H
