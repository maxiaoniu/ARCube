#-------------------------------------------------
#
# Project created by QtCreator 2015-04-14T19:02:17
#
#-------------------------------------------------

QT       += core gui opengl


TARGET = FinalPro
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    window.cpp \
    modelloader.cpp \
    model.cpp \
    camera.cpp \
    transform.cpp \
    backgroud.cpp \
    glbuffers.cpp \
    cubeface.cpp

HEADERS += \
    window.h \
    modelloader.h \
    model.h \
    camera.h \
    transform.h \
    backgroud.h \
    glbuffers.h \
    cubeface.h



macx: LIBS += -L$$PWD/../../../../../../usr/local/Cellar/assimp/3.1.1/lib/ -lassimp

INCLUDEPATH += $$PWD/../../../../../../usr/local/include/assimp
DEPENDPATH += $$PWD/../../../../../../usr/local/include/assimp

RESOURCES += \
    shader.qrc

DISTFILES +=

macx: LIBS += -L$$PWD/../../../../../../usr/local/Cellar/opencv/2.4.10.1/lib/ -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_calib3d

INCLUDEPATH += $$PWD/../../../../../../usr/local/Cellar/opencv/2.4.10.1/include
DEPENDPATH += $$PWD/../../../../../../usr/local/Cellar/opencv/2.4.10.1/include

macx: LIBS += -L$$PWD/../../../../../../usr/local/Cellar/Leapmotion/lib/ -lLeap

INCLUDEPATH += $$PWD/../../../../../../usr/local/Cellar/Leapmotion/include
DEPENDPATH += $$PWD/../../../../../../usr/local/Cellar/Leapmotion/include
