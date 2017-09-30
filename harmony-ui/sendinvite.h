#ifndef SENDINVITE_H
#define SENDINVITE_H

#include <QDialog>

namespace Ui {
class SendInvite;
}

class SendInvite : public QDialog
{
    Q_OBJECT

public:
    explicit SendInvite(QWidget *parent = 0);
    ~SendInvite();

private:
    Ui::SendInvite *ui;
};

#endif // SENDINVITE_H
