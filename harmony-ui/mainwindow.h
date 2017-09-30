#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
<<<<<<< HEAD
#include <algorithm>
#include <cctype>
#include <locale>
=======
>>>>>>> 7c1c6705ff147e168804e00581d887d6ff409818

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_EnterButton_pressed();
<<<<<<< HEAD
    void on_MessageInput_returnPressed();
=======
    void on_MessageInput_textChanged();
>>>>>>> 7c1c6705ff147e168804e00581d887d6ff409818

private:
    Ui::MainWindow *ui;
    void post_message();
};

#endif // MAINWINDOW_H
