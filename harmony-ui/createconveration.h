#ifndef CREATECONVERATION_H
#define CREATECONVERATION_H

#include <QDialog>

namespace Ui {
class CreateConveration;
}

class CreateConveration : public QDialog
{
    Q_OBJECT

public:
    explicit CreateConveration(QWidget *parent = 0);
    ~CreateConveration();

private:
    Ui::CreateConveration *ui;
};

#endif // CREATECONVERATION_H
