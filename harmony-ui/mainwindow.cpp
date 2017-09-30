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

<<<<<<< HEAD
/**
 * @brief MainWindow::on_EnterButton_pressed reacts to the enter button being
 * pressed. It should attempt to post the message held in MessageInput's
 * buffer.
 */
=======
>>>>>>> 7c1c6705ff147e168804e00581d887d6ff409818
void MainWindow::on_EnterButton_pressed()
{
    post_message();
}

<<<<<<< HEAD
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
=======
void MainWindow::on_MessageInput_textChanged()
{
    QString text = ui->MessageInput->toPlainText();
    int textLength = text.length();
    if(textLength != 0) {
        if(text.at(textLength - 1) == "\n"){
            post_message();
        }
    }
}

void MainWindow::post_message()
{
    ui->MessageHistory->append(ui->MessageInput->toPlainText());
>>>>>>> 7c1c6705ff147e168804e00581d887d6ff409818
    ui->MessageInput->clear();
}
