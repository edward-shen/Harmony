#pragma once
#include "mainwindow.h"

extern MainWindow* g_main_win;

void qt_init(int argc, char *argv[]);
int qt_run();