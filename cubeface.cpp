#include "cubeface.h"

#include <QVector3D>
#include <QDebug>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
CubeFace::CubeFace()
{
 m_buffer = new QOpenGLBuffer();
 m_buffer->create();
 m_buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
 m_vao = new QOpenGLVertexArrayObject();
 m_vao->create();
}
//clockwise 4 vertex uv TL TR BL BR
void CubeFace::create(float x1,float y1, float x2,float y2,float x3,float y3,float x4,float y4)
{
    QOpenGLFunctions f = QOpenGLFunctions(QOpenGLContext::currentContext());

    //creat the vertexes
    float CubeFaceVertices[] =
    {
        // Positions // Texture Coords

         1,-1,  0,  x3, y3,
         1, 1,  0,  x2, y1,
        -1, 1,  0,  x1, y1,

        -1, 1,  0,  x1, y1,
        -1,-1,  0,  x4, y4,
         1,-1,  0,  x3, y3,

    };
    m_buffer->bind();
    m_buffer->allocate(CubeFaceVertices, sizeof(CubeFaceVertices));
    m_vao->bind();
    f.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    f.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    f.glEnableVertexAttribArray(0);
    f.glEnableVertexAttribArray(1);
    m_vao->release();
    m_buffer->release();
}

void CubeFace::draw()
{
    m_vao->bind();
    QOpenGLFunctions f = QOpenGLFunctions(QOpenGLContext::currentContext());
    f.glDrawArrays(GL_TRIANGLES, 0, 12);
    m_vao->release();

}

CubeFace::~CubeFace()
{
    delete m_buffer;
    delete m_vao;
}

