#include "qt_main.h"
#include "mainwindow.h"
#include <QApplication>

MainWindow* g_main_win;

int qt_main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    g_main_win = &w;
    w.show();

    return a.exec();
}