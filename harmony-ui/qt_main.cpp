#include "qt_main.h"
#include "mainwindow.h"
#include "qt_data.h"
#include <QApplication>

MainWindow* g_main_win;

QApplication* a;

void qt_init(int argc, char *argv[]) {
    a = new QApplication(argc, argv);
    qRegisterMetaType<QCustomData>();
    g_main_win = new MainWindow();
    g_main_win->show();
}

int qt_run() {
    return a->exec();
}