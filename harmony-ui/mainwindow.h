#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <memory>

#include "../encrypt.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void recieve_conversation_invite();
    void recieve_plaintext(harmony::conv::conv_message* msg);

private slots:
    void on_EnterButton_pressed();
    void on_MessageInput_returnPressed();

private:
    Ui::MainWindow *ui;
    void post_message();
    std::string username = "test";
};

#endif // MAINWINDOW_H
