#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../event.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::on_EnterButton_pressed reacts to the enter button being
 * pressed. It should attempt to post the message held in MessageInput's
 * buffer.
 */
void MainWindow::on_EnterButton_pressed()
{
    post_message();
}

/**
 * @brief MainWindow::on_MessageInput_returnPressed checks if the string is
 * empty. If so, do nothing. Else, call the post routine.
 */
void MainWindow::on_MessageInput_returnPressed()
{
    QString text = ui->MessageInput->text();
    if(text.trimmed() != "")
    {
        post_message();
    }
}

/**
 * @brief MainWindow::post_message appends the history log with the text stored
 * in the MessageInput buffer. It then clears the buffer.
 */
void MainWindow::post_message()
{
    QString text = ui->MessageInput->text();
    std::string* heap_text = new std::string(text.toUtf8().constData());

    ui->MessageHistory->append(QString(username += text));
    ui->MessageInput->clear();

    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::SEND_PLAINTEXT, heap_text));
}
