#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_O_triggered();

    void on_action_About_triggered();

    void on_action_Login_triggered();

    void on_action_X_triggered();

    void on_pushButtonPlayPause_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
