#include "login.h"
#include "ui_login.h"

#include "theme.h"

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>

login::login(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::login)
{
    ui->setupUi(this);

    setObjectName(QStringLiteral("loginDialog"));
    setWindowTitle(tr("登录音乐魔盒"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint
                             & ~Qt::WindowMinimizeButtonHint);
    setModal(true);
    setStyleSheet(Theme::styleSheet());
    setFixedSize(size()); // 登录框内容固定，不允许拉伸

    m_verifyCode = setVerifyCode();

    ui->labelVerify->setText(QString::number(m_verifyCode));
    ui->labelVerify->setStyleSheet(QStringLiteral("color:%1; font-size:16pt; font-weight:700;")
                                       .arg(Theme::Accent));
    ui->labelVerify->setAlignment(Qt::AlignCenter);
    ui->lineEditVerify->setPlaceholderText(tr("请输入右侧验证码"));

    ui->pushButtonLogin->setObjectName(QStringLiteral("playButton"));
    ui->pushButtonLogin->setCursor(Qt::PointingHandCursor);
    ui->pushButtonCancle->setObjectName(QStringLiteral("glassButton"));
    ui->pushButtonCancle->setCursor(Qt::PointingHandCursor);

    ui->lineEditUser->setFocus();
}

login::~login()
{
    delete ui;
}

int login::setVerifyCode()
{
    int code = 0;
    for (int i = 0; i < 4; ++i)
        code = code * 10 + QRandomGenerator::global()->bounded(10);
    return code;
}

void login::on_pushButtonLogin_clicked()
{
    if (!checkVerifyCode())
        return;

    // TODO: 在此接入真实账号校验
    accept();
}

void login::on_pushButtonCancle_clicked()
{
    reject();
}

bool login::checkVerifyCode()
{
    const QString input = ui->lineEditVerify->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请输入验证码！"));
        ui->lineEditVerify->setFocus();
        return false;
    }

    bool ok = false;
    const int value = input.toInt(&ok);
    if (!ok || value != m_verifyCode) {
        QMessageBox::warning(this, tr("提示"), tr("验证码输入错误，请重试！"));
        m_verifyCode = setVerifyCode();
        ui->labelVerify->setText(QString::number(m_verifyCode));
        ui->lineEditVerify->clear();
        ui->lineEditVerify->setFocus();
        return false;
    }

    return true;
}
