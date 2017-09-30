#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../event.h"
#include "conversationinviteaccept.h"
#include <qthread.h>

extern MainWindow* g_main_win;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QThread *thread = new QThread(this);
    updater = new GUIUpdater();
    updater->moveToThread(thread);
    connect(updater, SIGNAL(_appendChatText(QString)), this, SLOT(appendChatText(QString)));
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
    ui->MessageHistory->append(text);
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

void MainWindow::recieve_conversation_invite(harmony::conv::invite_notification* inv) {
    //CIAWindow = new CIAWindow(); // Be sure to destroy your window somewhere
    //CIAWindow->show();
}

void MainWindow::on_actionQuit_triggered() {
    this->close();
}

void MainWindow::on_action_Settings_triggered()
{

}

void MainWindow::on_actionCreate_triggered()
{

}

void MainWindow::on_actionInvite_triggered()
{

}

void MainWindow::on_actionLeave_triggered()
{

}
