#include "Backgroud.h"
#include <QVector3D>
#include <QDebug>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
Backgroud::Backgroud()
{
    m_width = 1.0;
}
void Backgroud::create()
{
    m_buffer = new QOpenGLBuffer();
    m_buffer->create();
    m_buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vao = new QOpenGLVertexArrayObject();
    m_vao->create();

    QOpenGLFunctions f = QOpenGLFunctions(QOpenGLContext::currentContext());

    //creat the vertexes
    float backgroudVertices[] =
    {
        // Positions       // Texture Coords
        //front
        0,    0,    0,      1, 1,
        1280, 0,    0,      0, 1,
        1280, 720,  0,      0, 0,

        1280, 720, 0,       0, 0,
        0,    720, 0,       1, 0,
        0,    0,   0,       1, 1
        //back
//        m_width, -m_width/2, -m_width, 0.0f, 0.0f,
//        m_width, -m_width/2,  m_width, 0.0f, 1.0f,
//       -m_width, -m_width/2,  m_width, 1.0f, 1.0f,

//       -m_width, -m_width/2, -m_width, 1.0f, 0.0f,
//        m_width, -m_width/2, -m_width, 0.0f, 0.0f,
//       -m_width, -m_width/2,  m_width, 1.0f, 1.0f
    };
    m_buffer->bind();
    m_buffer->allocate(backgroudVertices, sizeof(backgroudVertices));
    m_vao->bind();
    f.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    f.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    f.glEnableVertexAttribArray(0);
    f.glEnableVertexAttribArray(1);

    m_vao->release();
    m_buffer->release();
}

void Backgroud::draw()
{
    m_vao->bind();
    QOpenGLFunctions f = QOpenGLFunctions(QOpenGLContext::currentContext());
    int vPort[4]; f.glGetIntegerv(GL_VIEWPORT, vPort);
    f.glDrawArrays(GL_TRIANGLES, 0, 12);
    m_vao->release();

}

Backgroud::~Backgroud()
{
    delete m_buffer;
    delete m_vao;

}

