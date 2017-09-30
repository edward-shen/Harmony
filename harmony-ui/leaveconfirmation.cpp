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

void LeaveConfirmation::on_buttonBox_accepted()
{

}

void LeaveConfirmation::on_buttonBox_rejected()
{
    this->close();
}
