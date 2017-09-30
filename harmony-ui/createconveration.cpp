#include "createconveration.h"
#include "ui_createconveration.h"

CreateConveration::CreateConveration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateConveration)
{
    ui->setupUi(this);
}

CreateConveration::~CreateConveration()
{
    delete ui;
}
