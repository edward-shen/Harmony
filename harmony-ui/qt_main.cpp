#include "qt_main.h"
#include "mainwindow.h"
#include "qt_data.h"
#include <QApplication>

MainWindow* g_main_win;

QApplication* a;

/**
 * @brief qt_init initializes the Qt window
 * @param argc command line args passed from main.cpp
 * @param argv command line values passed from main.cpp
 */
void qt_init(int argc, char *argv[]) {
    a = new QApplication(argc, argv);
    qRegisterMetaType<QCustomData>();
    g_main_win = new MainWindow();
    g_main_win->show();
}

/**
 * @brief qt_run starts and shows the Qt windows
 * @return exit code
 */
int qt_run() {
    return a->exec();
}
