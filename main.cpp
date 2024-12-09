//添加跨编译器的代码
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "mymusicplayer.h"
#include <QApplication>
#include <QScreen>


#include "QTranslator"

int main(int argc, char *argv[])
{
    //创建QApplication对象，管理整个应用程序的资源，通过argc和argv来获取它自己的命令行参数；
    QApplication a(argc, argv);

    //加载Qt中的资源文件，使Qt显示中文（包括QMessageBox、文本框右键菜单等）
    QTranslator translator;
    if (!translator.load(":/resources/translations/qt_zh_CN.qm")) {
        qWarning() << "无法加载翻译文件";
        // 可以在这里添加额外的错误处理逻辑
    }
    a.installTranslator(&translator);

//    MainWindow w;
//    w.show();
//如何关闭主窗口的同时关闭所有的窗口？


    myMusicPlayer *mp = new myMusicPlayer();
    //把窗口居中显示
    QScreen *screen = QApplication::primaryScreen();
    mp->move((screen->geometry().width() - mp->width())/2, (screen->geometry().height() - mp->height())/2);
    mp->show();

    return a.exec();
}
