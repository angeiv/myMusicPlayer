#include "mymusicplayer.h"
#include <QApplication>
#include <QDesktopWidget>


#include "QTranslator"

int main(int argc, char *argv[])
{
    //创建QApplication对象，管理整个应用程序的资源，通过argc和argv来获取它自己的命令行参数；
    QApplication a(argc, argv);

    //加载Qt中的资源文件，使Qt显示中文（包括QMessageBox、文本框右键菜单等）
    QTranslator translator;
    translator.load(":/resources/translations/qt_zh_CN");
    a.installTranslator(&translator);

//    MainWindow w;
//    w.show();
//如何关闭主窗口的同时关闭所有的窗口？


    myMusicPlayer *mp = new myMusicPlayer();
    //把窗口居中显示
    mp->move((QApplication::desktop()->width() - mp->width())/2,(QApplication::desktop()->height() - mp->height())/2);
    mp->show();

    return a.exec();
}
