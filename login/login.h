#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();
    int setVerifyCode();

private slots:

    void on_pushButtonLogin_clicked();

    void on_pushButtonCancle_clicked();

    bool checkVerifyCode();//检查验证码

private:
    Ui::login *ui;
};

#endif // LOGIN_H
