#include "sendinvite.h"
#include "ui_sendinvite.h"

SendInvite::SendInvite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendInvite)
{
    ui->setupUi(this);
}

SendInvite::~SendInvite()
{
    delete ui;
}
