#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <memory>

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
    void on_MessageInput_returnPressed();

private:
    Ui::MainWindow *ui;
    void post_message();
};

#endif // MAINWINDOW_H
