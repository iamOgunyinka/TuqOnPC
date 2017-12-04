#-------------------------------------------------
#
# Project created by QtCreator 2017-08-07T17:45:37
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TuqOnThePC
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


SOURCES += main.cpp\
    mainwindow.cpp \
    add_question_window.cpp \
    question_dialog.cpp \
    repository_lister.cpp \
    upload_status_dialog.cpp \
    score_sheet_window.cpp \
    preload_window.cpp \
    sign_in_dialog.cpp \
    list_courses_dialog.cpp

HEADERS  += \
    mainwindow.hpp \
    add_question_window.hpp \
    question_dialog.hpp \
    repository_lister.hpp \
    upload_status_dialog.hpp \
    resources.hpp \
    score_sheet_window.hpp \
    preload_window.hpp \
    sign_in_dialog.hpp \
    list_courses_dialog.hpp

FORMS    += \
    add_question_window.ui \
    question_dialog.ui \
    score_sheet_window.ui \
    sign_in_dialog.ui \
    list_courses_dialog.ui

RESOURCES += \
    images.qrc
