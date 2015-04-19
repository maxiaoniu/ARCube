#ifndef MODEL_H
#define MODEL_H
#include <QOpenGLBuffer>
#include <QVector3D>
class QOpenGLVertexArrayObject;

class Model
{
public:
    Model();
    ~Model();

    QOpenGLBuffer *getVBO();
    QOpenGLBuffer *getEBO();
    QOpenGLVertexArrayObject *getVertexArrayObject();
    void createBuffer(QOpenGLBuffer::UsagePattern hint);
    void createVertexArrayObjct();
    void setDrawArrays(int count);
    void setDrawMode(GLenum mode);
    void setOritation(GLenum mode);
    void draw();
private:
    QOpenGLBuffer *m_vbo;
    QOpenGLBuffer *m_ebo;
    QOpenGLVertexArrayObject *m_vao;
    GLenum m_mode;
    int m_count;
    GLenum m_oritation;
};

#endif // MODEL_H
