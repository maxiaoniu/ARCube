#include "model.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QDebug>
#include <QString>
Model::Model()
{
    m_mode = GL_LINES;
    m_oritation = GL_CCW;
}

Model::~Model()
{
    if(m_vbo)
        delete m_vbo;
    if(m_ebo)
        delete m_ebo;
    if(m_vao)
        delete m_vao;
}

void Model::createBuffer(QOpenGLBuffer::UsagePattern hint)
{

    m_vbo = new QOpenGLBuffer();
    m_vbo->create();
    m_vbo->setUsagePattern(hint);

    m_ebo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_vbo->create();
    m_vbo->setUsagePattern(hint);

}

QOpenGLBuffer* Model::getVBO()
{
    return m_vbo;
}

QOpenGLBuffer* Model::getEBO()
{
    return m_ebo;
}

void Model::createVertexArrayObjct()
{
    m_vao = new QOpenGLVertexArrayObject();
    m_vao->create();
}

QOpenGLVertexArrayObject *Model::getVertexArrayObject()
{
    return m_vao;

}

void Model::setDrawArrays(int count)
{
    m_count = count;
}

void Model::setDrawMode(GLenum mode)
{
    m_mode = mode;
}

void Model::setOritation(GLenum mode)
{
    m_oritation = mode;
}

void Model::draw()
{
    m_vbo->bind();
    m_ebo->bind();
    m_vao->bind();

    QOpenGLFunctions f(QOpenGLContext::currentContext());

    f.glFrontFace(m_oritation);
    //f.glDrawArrays(m_mode, 0, m_count);
    glDrawElements(m_mode, m_count, GL_UNSIGNED_INT, 0);
    m_vao->release();
    m_vbo->release();
    m_ebo->release();

}


