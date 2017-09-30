#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../event.h"
#include "conversationinviteaccept.h"
#include <qthread.h>
#include <QMessageBox>
#include <iostream>

extern MainWindow* g_main_win;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QThread *thread = new QThread(this);
    updater = new GUIUpdater();
    updater->moveToThread(thread);
    connect(updater, SIGNAL(_appendChatText(QString)), this, SLOT(appendChatText(QString)));
    connect(updater, SIGNAL(_setUsers(QStringList)), this, SLOT(setUsers(QStringList)));
    connect(updater, SIGNAL(_promptInvite(QCustomData)), this, SLOT(promptInvite(QCustomData)));
    connect(thread, SIGNAL(destroyed()), updater, SLOT(deleteLater()));

    CIAWindow = new ConversationInviteAccept();
}

MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief MainWindow::on_EnterButton_pressed reacts to the enter button being
 * pressed. It should attempt to post the message held in MessageInput's
 * buffer.
 */
void MainWindow::on_EnterButton_pressed() {
    post_message();
}

/**
 * @brief MainWindow::on_MessageInput_returnPressed checks if the string is
 * empty. If so, do nothing. Else, call the post routine.
 */
void MainWindow::on_MessageInput_returnPressed() {
    post_message();
}

void MainWindow::appendChatText(const QString& text) {
    ui->MessageHistory->append(text);
}

void MainWindow::setUsers(const QStringList& list) {
    ui->UserList->clear();
    ui->UserList->insertItems(0, list);
}

bool hasEnding(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void MainWindow::post_message() {
    QString text = ui->MessageInput->text();
    if (text.trimmed() != "") {
        harmony::conv::conv_message* msg = new harmony::conv::conv_message(harmony::conv::default_conv(),
            harmony::conv::my_username(),
            std::string(text.toUtf8().constData()));
        harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::SEND_PLAINTEXT, msg));
    }

    ui->MessageInput->clear();
}

void MainWindow::recieve_plaintext(harmony::conv::conv_message* msg) {
    g_main_win->updater->appendChatText(QString::fromStdString(msg->sender + ": ") + QString::fromStdString(msg->message));
}

void MainWindow::recieve_user_list(std::vector<std::string>* users) {
    QStringList list;
    std::string me = harmony::conv::my_username();
    for (auto it = users->begin(); it != users->end(); ++it) {
        std::string user = *it;
        if (user == me) user = user + " (YOU)";
        list << QString::fromStdString(user);
    }
    g_main_win->updater->setUsers(list);
}

void MainWindow::recieve_conversation_invite(harmony::conv::invite_notification* inv) {
    std::cout << "Pointer going in " << ((uintptr_t) inv) << std::endl;
    harmony::conv::invite_notification* inv2 = new harmony::conv::invite_notification(*inv);
    QCustomData data(inv2);
    g_main_win->updater->promptInvite(data);
}

void MainWindow::promptInvite(const QCustomData& from) {
    harmony::conv::invite_notification* inv = (harmony::conv::invite_notification*) from.ptr();
    std::cout << "Pointer coming out " << ((uintptr_t) inv) << std::endl;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, QString::fromStdString("Invite from " + inv->from), "Accept invite?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::JOIN_CONV, inv));
    }
}

void MainWindow::on_actionQuit_triggered() {
    this->close();
}

void MainWindow::on_action_Settings_triggered() {

}

void MainWindow::on_actionCreate_triggered() {

}

void MainWindow::on_actionInvite_triggered() {
    QListWidgetItem* itm = ui->UserList->currentItem();
    std::string name(itm->text().toUtf8().constData());
    if (hasEnding(name, " (YOU)")) return;
    harmony::conv::invite_out* inv = new harmony::conv::invite_out(name, harmony::conv::default_conv());
    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::SEND_INVITE, inv));
}

void MainWindow::on_actionLeave_triggered() {

}
