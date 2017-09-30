#include "conversationinviteaccept.h"
#include "ui_conversationinviteaccept.h"

ConversationInviteAccept::ConversationInviteAccept(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConversationInviteAccept)
{
    ui->setupUi(this);
}

ConversationInviteAccept::~ConversationInviteAccept()
{
    delete ui;
}

void ConversationInviteAccept::on_buttonBox_accepted()
{

}

void ConversationInviteAccept::on_buttonBox_rejected()
{

}
