#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../event.h"
#include "conversationinviteaccept.h"
#include <qthread.h>
#include <QMessageBox>
#include <QInputDialog>
#include <iostream>
#include <unordered_map>
#include <sstream>

extern MainWindow* g_main_win;

std::string current_channel;
std::unordered_map<std::string, std::ostringstream*> conv_map;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // We need to separate our UI updating thread from the event handler thread.
    QThread *thread = new QThread(this);
    updater = new GUIUpdater();
    updater->moveToThread(thread);

    // Create our Qt Signals and Slots! No one apparently knows what they actually do, but
    // Given the syntax, it looks like they pass the output of the signal into the slot function.
    connect(updater, SIGNAL(_appendChatText(QString)), this, SLOT(appendChatText(QString)));
    connect(updater, SIGNAL(_setUsers(QStringList)), this, SLOT(setUsers(QStringList)));
    connect(updater, SIGNAL(_displayConvList(QStringList)), this, SLOT(displayConvList(QStringList)));
    connect(updater, SIGNAL(_promptInvite(QCustomData)), this, SLOT(promptInvite(QCustomData)));
    connect(thread, SIGNAL(destroyed()), updater, SLOT(deleteLater()));
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

/**
 * @brief MainWindow::appendChatText appends the message to the history
 * @param text chat to be appended to the message history
 */
void MainWindow::appendChatText(const QString& text) {
    ui->MessageHistory->setText(text);
}

/**
 * @brief MainWindow::setUsers clears the current users in the userlist, and
 * replaces it with the provided list
 * @param list list of users to be set
 */
void MainWindow::setUsers(const QStringList& list) {
    ui->UserList->clear();
    ui->UserList->insertItems(0, list);
}

/**
 * @brief hasEnding returns whether or not fullString has ending as their ending
 * @param fullString Larger string that should be checked for the ending
 * @param ending string to check for
 * @return true if fullString ends with ending.
 */
bool hasEnding(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

/**
 * @brief MainWindow::post_message If the message is valid, adds the message to the queue and clears
 * the input buffer.
 */
void MainWindow::post_message() {
    QString text = ui->MessageInput->text();
    if (text.trimmed() != "") {
        // Sets the conversation to the default one if there isn't one yet
        if (current_channel.size() == 0) {
            current_channel = harmony::conv::default_conv();
        }

        // Packages the message into a conversation message, so we can add it to the queue.
        // This is better than directly assuming the message will be sent. Doing it like this
        // ensures that the message history is the correct order.
        harmony::conv::conv_message* msg = new harmony::conv::conv_message(current_channel,\
            harmony::conv::my_username(),
            std::string(text.toUtf8().constData()));
        harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::SEND_PLAINTEXT, msg));
    }

    ui->MessageInput->clear();
}

/**
 * @brief MainWindow::recieve_plaintext
 * @param msg
 */
void MainWindow::recieve_plaintext(harmony::conv::conv_message* msg) {
    if (conv_map.find(msg->conv) == conv_map.end()) {
        throw std::runtime_error("Recieved malformed data!");
    }
    std::ostringstream* out = conv_map[msg->conv];
    *out << msg->sender << ": " << msg->message << '\n';

    if (current_channel.size() == 0 || conv_map.find(current_channel) == conv_map.end())
        return;

    g_main_win->updater->appendChatText(QString::fromStdString(conv_map[current_channel]->str()));
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
    harmony::conv::invite_notification* inv2 = new harmony::conv::invite_notification(*inv);
    QCustomData data(inv2);
    g_main_win->updater->promptInvite(data);
}

void MainWindow::recieve_conversation_joined(std::string& name) {
    std::ostringstream* os = new std::ostringstream();
    conv_map[name] = os;
    QStringList list;
    for (auto it = conv_map.begin(); it != conv_map.end(); ++it) {
        list << QString::fromStdString(it->first);
    }
    g_main_win->updater->displayConvList(list);
}

void MainWindow::promptInvite(const QCustomData& from) {
    harmony::conv::invite_notification* inv = (harmony::conv::invite_notification*) from.ptr();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, QString::fromStdString("Invite from " + inv->from), "Accept invite?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::JOIN_CONV, inv));
    }
}

void MainWindow::displayConvList(const QStringList& list) {
    ui->ConvList->clear();
    ui->ConvList->insertItems(0, list);
}

/**
 * @brief MainWindow::on_actionQuit_triggered
 * This is called from the top menu's quit button.
 * By closing this window, we initiate the closing of the rest of the threads.
 */
void MainWindow::on_actionQuit_triggered() {
    this->close();
}

void MainWindow::on_action_Settings_triggered() {

}

/**
 * @brief MainWindow::on_actionCreate_triggered
 * This is called from the top menu's Create converastion button and places the
 * user into a new conversation.
 */
void MainWindow::on_actionCreate_triggered() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("Conversation Name:"), QLineEdit::Normal,
                                         "hi", &ok);
    if (ok && !text.isEmpty())
    {}
    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::MAKE_CONV, nullptr));
}

/**
 * @brief MainWindow::on_actionInvite_triggered
 * This is called from the top menu's Invite conversation button and, if the
 * user selection is valid, invites the user to another conversation.
 */
void MainWindow::on_actionInvite_triggered() {
    QListWidgetItem* itm = ui->UserList->currentItem();

    // You cannot reinvite the same person sucessively
    if (itm == nullptr) { return; }

    std::string name(itm->text().toUtf8().constData());
    // You cannot invite yourself
    if (hasEnding(name, " (YOU)")) { return; }

    std::string conv = current_channel;

    // If no conv exists, use the default one
    if (conv.size() == 0) { conv = harmony::conv::default_conv(); }

    harmony::conv::invite_out* inv = new harmony::conv::invite_out(name, conv);
    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::SEND_INVITE, inv));
}

void MainWindow::on_actionLeave_triggered() {

}

void MainWindow::on_ConvList_currentRowChanged(int currentRow)
{

}
