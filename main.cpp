#include "mymusicplayer.h"
#include "theme.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QTranslator>

int main(int argc, char *argv[])
{
    // 高分屏按真实像素绘制，避免图标发虚（Qt 6 默认即 PassThrough，这里显式声明）
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("MusicBox"));
    QApplication::setApplicationName(QStringLiteral("音乐魔盒"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    // Fusion 是唯一能被 QSS 完整接管的跨平台基础样式，必须在创建窗口前设置
    Theme::applyApplicationTheme();

    // 系统自带对话框/菜单的中文翻译
    static QTranslator qtTranslator;
    if (qtTranslator.load(QStringLiteral(":/resources/translations/qt_zh_CN.qm")))
        QApplication::installTranslator(&qtTranslator);
    else
        qWarning("无法加载 qt_zh_CN.qm，Qt 自带对话框将使用英文");

    myMusicPlayer player;
    player.show();

    // 在屏幕可用区域内居中；窗口大于屏幕时自动夹紧
    if (const QScreen *screen = QApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        player.resize(player.size().boundedTo(available.size() - QSize(40, 40)));
        player.move(available.center() - QPoint(player.width() / 2, player.height() / 2));
    }

    return QApplication::exec();
}
