#-------------------------------------------------
#
# Project created by QtCreator 2017-09-29T21:29:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = harmony-ui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        harmony-ui/qt_main.cpp \
        harmony-ui/mainwindow.cpp \
    event.cpp \
    harmony-ui/conversationinviteaccept.cpp \
    main.cpp \
    harmony-ui/sendinvite.cpp \
    harmony-ui/leaveconfirmation.cpp

HEADERS += \
        harmony-ui/mainwindow.h \
    event.h \
    harmony-ui/conversationinviteaccept.h \
    harmony-ui/sendinvite.h \
    harmony-ui/leaveconfirmation.h

FORMS += \
        harmony-ui/mainwindow.ui \
    harmony-ui/conversationinviteaccept.ui \
    harmony-ui/sendinvite.ui \
    harmony-ui/leaveconfirmation.ui

CONFIG += c++14

DISTFILES += \
    CMakeLists.txt
