#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

class Transform
{
public:
    static const QVector3D LocalForward;
    static const QVector3D LocalUp;
    static const QVector3D LocalRight;

    Transform();
    ~Transform();

    void translate(const QVector3D &dt);
    void scale(const QVector3D &ds);
    void scale(const float factor);
    void rotate(float angle, const QVector3D &axis);
    void rotate(const QQuaternion &rotation);

    const QVector3D& getTranslation();
    const QVector3D& getScale();
    const QQuaternion& getRotation();
    const QMatrix4x4& getMatrix();

    void setTranslation(const QVector3D &t);
    void setTranslation(float x, float y, float z);
    void setScale(const QVector3D &s);
    void setRotation(const QQuaternion &r);
    void setRotation(float angle, float ax, float ay, float az);
    void setToIdentity();

    QVector3D forward();
    QVector3D up();
    QVector3D right();

private:
    bool m_dirty;
    QVector3D m_translation;
    QVector3D m_scale;
    QQuaternion m_rotation;
    QMatrix4x4 m_model2world;


};

#endif // TRANSFORM_H
