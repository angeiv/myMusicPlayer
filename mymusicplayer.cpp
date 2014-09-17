#include "mymusicplayer.h"
#include "ui_mymusicplayer.h"

mymusicplayer::mymusicplayer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mymusicplayer)
{
    ui->setupUi(this);
}

mymusicplayer::~mymusicplayer()
{
    delete ui;
}
