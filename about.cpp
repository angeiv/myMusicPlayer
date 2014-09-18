#include "about.h"
#include "ui_about.h"
#include "QDesktopServices"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);//去除问号按钮
}

About::~About()
{
    delete ui;
}

void About::on_labelPage_linkActivated(const QString &link)
{
    connect(ui->labelPage, SIGNAL(linkActivated(QString)), this, SLOT(openUrl(QString)));
    this->ui->labelPage->setOpenExternalLinks(true);

}

void About::openUrl(QString url)
{
    QDesktopServices::openUrl(QUrl(url));
}
