/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTextBrowser *UserList;
    QTextBrowser *MessageHistory;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QPlainTextEdit *MessageInput;
    QPushButton *EnterButton;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(719, 641);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        UserList = new QTextBrowser(centralWidget);
        UserList->setObjectName(QStringLiteral("UserList"));
        UserList->setEnabled(true);
        UserList->setGeometry(QRect(459, 10, 241, 571));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(UserList->sizePolicy().hasHeightForWidth());
        UserList->setSizePolicy(sizePolicy);
        UserList->setMinimumSize(QSize(200, 0));
        UserList->setMaximumSize(QSize(400, 16777215));
        MessageHistory = new QTextBrowser(centralWidget);
        MessageHistory->setObjectName(QStringLiteral("MessageHistory"));
        MessageHistory->setEnabled(true);
        MessageHistory->setGeometry(QRect(10, 10, 441, 571));
        widget = new QWidget(centralWidget);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(10, 580, 691, 86));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        MessageInput = new QPlainTextEdit(widget);
        MessageInput->setObjectName(QStringLiteral("MessageInput"));
        sizePolicy.setHeightForWidth(MessageInput->sizePolicy().hasHeightForWidth());
        MessageInput->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(MessageInput);

        EnterButton = new QPushButton(widget);
        EnterButton->setObjectName(QStringLiteral("EnterButton"));

        horizontalLayout->addWidget(EnterButton);

        MainWindow->setCentralWidget(centralWidget);
        MessageInput->raise();
        EnterButton->raise();
        MessageHistory->raise();
        UserList->raise();
        MessageInput->raise();
        UserList->raise();
        MessageHistory->raise();

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        EnterButton->setText(QApplication::translate("MainWindow", "Enter", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
