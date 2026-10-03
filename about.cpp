#include "about.h"
#include "ui_about.h"
#include "theme.h"

#include <QLabel>

About::About(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);

    setObjectName(QStringLiteral("aboutDialog"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setStyleSheet(Theme::styleSheet());

    if (QLabel *logo = ui->labelLogo) {
        const QPixmap pixmap = logo->pixmap();
        if (!pixmap.isNull()) {
            logo->setPixmap(pixmap.scaled(logo->size(), Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
        }
    }
}

About::~About()
{
    delete ui;
}

void About::on_labelPage_linkActivated(const QString &)
{
    // 交由系统默认浏览器打开，不在 QLabel 内部渲染
    ui->labelPage->setOpenExternalLinks(true);
}
