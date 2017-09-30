#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <memory>

#include "../encrypt.h"
#include "conversationinviteaccept.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void recieve_conversation_invite(harmony::conv::invite_notification* inv );
    void recieve_plaintext(harmony::conv::conv_message* msg);

private slots:
    void on_EnterButton_pressed();
    void on_MessageInput_returnPressed();
    void on_actionQuit_triggered();

private:
    void post_message();
    Ui::MainWindow *ui;
    std::string username = "test";
    ConversationInviteAccept *CIAWindow;
};

#endif // MAINWINDOW_H
