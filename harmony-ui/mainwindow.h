#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <memory>

#include "qt_data.h"

#include "../encrypt.h"
#include "conversationinviteaccept.h"

namespace Ui {
class MainWindow;
}

class GUIUpdater : public QObject {
    Q_OBJECT

public:
    explicit GUIUpdater(QObject *parent = 0) : QObject(parent) {}
    void appendChatText(const QString &image) {
        emit _appendChatText(image);
    }
    void setUsers(const QStringList& list) {
        emit _setUsers(list);
    }
    void promptInvite(const QCustomData& from) {
        emit _promptInvite(from);
    }
    void displayConvList(const QStringList& list) {
        emit _displayConvList(list);
    }

signals:
    void _appendChatText(const QString&);
    void _setUsers(const QStringList&);
    void _promptInvite(const QCustomData&);
    void _displayConvList(const QStringList&);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static void recieve_conversation_invite(harmony::conv::invite_notification* inv);
    static void recieve_plaintext(harmony::conv::conv_message* msg);
    static void recieve_user_list(std::vector<std::string>* users);
    static void recieve_conversation_joined(std::string& name);


private slots:
    void on_EnterButton_pressed();
    void on_MessageInput_returnPressed();
    void on_actionQuit_triggered();
    void on_action_Settings_triggered();
    void on_actionCreate_triggered();
    void on_actionInvite_triggered();
    void on_actionLeave_triggered();

    void appendChatText(const QString&);
    void setUsers(const QStringList&);
    void promptInvite(const QCustomData&);
    void displayConvList(const QStringList&);
    void on_ConvList_currentRowChanged(int currentRow);

    void on_actionSet_Username_triggered();

private:
    void post_message();
    Ui::MainWindow *ui;
    std::string username = "test";
    ConversationInviteAccept *CIAWindow;

    GUIUpdater* updater;
};

#endif // MAINWINDOW_H
