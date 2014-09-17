#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "about.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_O_triggered()
{
/*    QString filename;
    filename = QFileDialog::getOpenFileName(this,tr("选择音乐"),"",tr("Music (*.mp3 *.wmv *.jpg"));

    if(filename.isEmpty()) {
        return;
    }
    else {
        QImage* img = new QImage;

        if(! (img->load(filename))) {
            QMessageBox::information(this,tr("提示"),tr("打开音乐失败！"));

            delete img;
            return;
        }
        ui->label->setPixmap(QPixmap::fromImage(*img));
    }*/
}

void MainWindow::on_action_About_triggered()
{
    About *ab = new About();
    ab->show();
}
