#-------------------------------------------------
#
# Project created by QtCreator 2015-05-05T23:17:34
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MakeItSpin
TEMPLATE = app


SOURCES += main.cpp\
    view/mainwindow.cpp \
    view/glwidget.cpp \
    model/objreader.cpp \
    view/mesh.cpp \
    view/shader.cpp

HEADERS  += view/mainwindow.h \
    view/glwidget.h \
    view/objreader.h \
    model/objreader.h \
    view/mesh.h \
    view/shader.h

FORMS    += view/mainwindow.ui