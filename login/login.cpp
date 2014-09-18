#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//去除最大最小化按钮
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);//去除问号按钮
}

login::~login()
{
    delete ui;
}
