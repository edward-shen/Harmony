#ifndef CONVERSATIONINVITEACCEPT_H
#define CONVERSATIONINVITEACCEPT_H

#include <QDialog>

namespace Ui {
class ConversationInviteAccept;
}

class ConversationInviteAccept : public QDialog
{
    Q_OBJECT

public:
    explicit ConversationInviteAccept(QWidget *parent = 0);
    ~ConversationInviteAccept();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::ConversationInviteAccept *ui;
};

#endif // CONVERSATIONINVITEACCEPT_H