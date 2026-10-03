#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class login;
}

/*!
 * \brief 登录对话框（演示用：仅验证码校验，未接入后端）。
 */
class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login() override;

    /// 生成 4 位数字验证码并返回其数值。
    int setVerifyCode();

private slots:
    void on_pushButtonLogin_clicked();
    void on_pushButtonCancle_clicked();

    /// 校验用户输入的验证码。
    bool checkVerifyCode();

private:
    Ui::login *ui = nullptr;
    int m_verifyCode = 0;
};

#endif // LOGIN_H
