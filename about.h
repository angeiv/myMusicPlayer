#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui {
class About;
}

/// 「关于音乐魔盒」对话框。
class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About() override;

private slots:
    /// 由 setupUi 自动连接：处理富文本中的超链接。
    void on_labelPage_linkActivated(const QString &link);

private:
    Ui::About *ui = nullptr;
};

#endif // ABOUT_H
