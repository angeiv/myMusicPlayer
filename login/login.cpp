#include "login.h"
#include "ui_login.h"
#include "QTime"
#include "QMessageBox"
#include <QRandomGenerator>

//验证码生成函数
int login::setVerifyCode()
{
    QString code = "";
    code += QString::number(QRandomGenerator::global()->bounded(10));
    code += QString::number(QRandomGenerator::global()->bounded(10));
    code += QString::number(QRandomGenerator::global()->bounded(10));
    code += QString::number(QRandomGenerator::global()->bounded(10));

    int VerifyCode = code.toInt();

    return VerifyCode;
}

//验证码
int VerifyCode;

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);//去除最大最小化按钮
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint);//去除问号按钮

    //生成验证码
    VerifyCode = setVerifyCode();
    //显示验证码，红色，20px
    QString vd;
    vd.setNum(VerifyCode);
    ui->labelVerify->setStyleSheet("color:red;font-size:20pt");
    ui->labelVerify->setText(vd);
    ui->lineEditUser->setFocus();
}

login::~login()
{
    delete ui;
}

void login::on_pushButtonLogin_clicked()
{
    //检查用户名和密码
    //检查验证码
    if( checkVerifyCode() ) {
        this->close();//密码正确，执行操作...
        //....
    }
}

void login::on_pushButtonCancle_clicked()
{
    this->close();
}

bool login::checkVerifyCode()
{
    if(ui->lineEditVerify->text().isEmpty()) { //未输入验证码，设置焦点
        QMessageBox::information(this,QString("提示"),QString("请输入验证码！"));
        ui->lineEditVerify->setFocus();
        return false;
    }

    else if(ui->lineEditVerify->text().toInt() != VerifyCode) { //验证码输入错误，设置焦点
        QMessageBox::warning(this,QString("提示"),QString("验证码输入错误，请重试！"));
        ui->lineEditVerify->setFocus();
        return false;
    }
    else {
        return true;
    }
}

QList<QUrl> playlist;
int currentIndex = 0;
