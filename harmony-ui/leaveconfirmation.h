#ifndef LEAVECONFIRMATION_H
#define LEAVECONFIRMATION_H

#include <QDialog>

namespace Ui {
class LeaveConfirmation;
}

class LeaveConfirmation : public QDialog
{
    Q_OBJECT

public:
    explicit LeaveConfirmation(QWidget *parent = 0);
    ~LeaveConfirmation();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::LeaveConfirmation *ui;
};

#endif // LEAVECONFIRMATION_H
