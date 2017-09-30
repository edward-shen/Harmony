#include "qt_main.h"
#include "mainwindow.h"
#include <QApplication>

int qt_main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}