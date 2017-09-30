#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    ui->MessageHistory->append(ui->MessageInput->text());
    ui->MessageInput->clear();
}
