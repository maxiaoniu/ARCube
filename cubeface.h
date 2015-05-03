#ifndef CUBEFACE_H
#define CUBEFACE_H

class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class CubeFace
{
public:
    CubeFace();
    ~CubeFace();
    void create(float x1,float y1, float x2,float y2,float x3,float y3,float x4,float y4);
    void draw();
private:
    QOpenGLBuffer *m_buffer;
    QOpenGLVertexArrayObject *m_vao;

};

#endif // CUBEFACE_H
