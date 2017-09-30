#include "leaveconfirmation.h"
#include "ui_leaveconfirmation.h"

LeaveConfirmation::LeaveConfirmation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LeaveConfirmation)
{
    ui->setupUi(this);
}

LeaveConfirmation::~LeaveConfirmation()
{
    delete ui;
}
