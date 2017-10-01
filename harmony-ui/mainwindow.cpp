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

    QThread *thread = new QThread(this);
    updater = new GUIUpdater();
    updater->moveToThread(thread);
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

void MainWindow::appendChatText(const QString& text) {
    ui->MessageHistory->setText(text);
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
    if (conv_map.find(msg->conv) == conv_map.end()) {
        throw std::runtime_error("wups");
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

void MainWindow::on_actionQuit_triggered() {
    this->close();
}

void MainWindow::on_action_Settings_triggered() {

}

void MainWindow::on_actionCreate_triggered() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("Conversation Name:"), QLineEdit::Normal,
                                         "hi", &ok);
    if (ok && !text.isEmpty())
    {}
    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::MAKE_CONV, nullptr));
}

void MainWindow::on_actionInvite_triggered() {
    QListWidgetItem* itm = ui->UserList->currentItem();
    if (itm == nullptr) return;
    std::string name(itm->text().toUtf8().constData());
    if (hasEnding(name, " (YOU)")) return;
    harmony::conv::invite_out* inv = new harmony::conv::invite_out(name, harmony::conv::default_conv());
    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::SEND_INVITE, inv));
}

void MainWindow::on_actionLeave_triggered() {

}

void MainWindow::on_ConvList_currentRowChanged(int currentRow)
{

}
