#-------------------------------------------------
#
# Project created by QtCreator 2015-04-14T19:02:17
#
#-------------------------------------------------

QT       += core gui


TARGET = FinalPro
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    window.cpp \
    modelloader.cpp \
    model.cpp \
    camera.cpp \
    transform.cpp

HEADERS += \
    window.h \
    modelloader.h \
    model.h \
    camera.h \
    transform.h



macx: LIBS += -L$$PWD/../../../../../../usr/local/Cellar/assimp/3.1.1/lib/ -lassimp

INCLUDEPATH += $$PWD/../../../../../../usr/local/include/assimp
DEPENDPATH += $$PWD/../../../../../../usr/local/include/assimp

RESOURCES += \
    shader.qrc
