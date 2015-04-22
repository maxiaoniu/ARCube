#ifndef BACKGROUD_H
#define BACKGROUD_H

class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class Backgroud
{
public:
    Backgroud();
    ~Backgroud();
    void create();
    void draw();
private:
    QOpenGLBuffer *m_buffer;
    QOpenGLVertexArrayObject *m_vao;
    float m_width;
};

#endif // BACKGROUD_H

