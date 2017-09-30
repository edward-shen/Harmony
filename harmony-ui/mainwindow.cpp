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

void MainWindow::on_EnterButton_pressed()
{
    post_message();
}

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
    ui->MessageInput->clear();
}