//添加跨编译器的代码
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>

#include "appcontroller.h"
#include "playerengine.h"

int main(int argc, char *argv[])
{
    //创建QApplication对象，管理整个应用程序的资源，通过argc和argv来获取它自己的命令行参数；
    QApplication app(argc, argv);

    //加载Qt中的资源文件，使Qt显示中文（包括QMessageBox、文本框右键菜单等）
    QTranslator translator;
    if (!translator.load(":/resources/translations/qt_zh_CN.qm")) {
        qWarning() << "无法加载翻译文件";
        // 可以在这里添加额外的错误处理逻辑
    }
    app.installTranslator(&translator);

    QCoreApplication::setOrganizationName("myMusicPlayer");
    QCoreApplication::setApplicationName("myMusicPlayer");
    QCoreApplication::setApplicationVersion("1.0");

    QGuiApplication::setWindowIcon(QIcon(":/resources/img/logo.ico"));

    qmlRegisterUncreatableType<PlayerEngine>("MyMusicPlayer", 1, 0, "PlayerEngine", "Enums only");

    PlayerEngine player;
    AppController controller(&player);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("player", &player);
    engine.rootContext()->setContextProperty("app", &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
