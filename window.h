#ifndef WINDOW_H
#define WINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "transform.h"
#include "camera.h"
#include <QMatrix4x4>
#include "model.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>

class Window : public QOpenGLWindow,
               protected QOpenGLFunctions
{
  Q_OBJECT

// OpenGL Events
public:
    Window();
  ~Window();

  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();
  void teardownGL();

private:
  // Private Helpers
  void printContextInformation();
  QOpenGLShaderProgram *m_program;

  int u_modelToWorld;
  int u_worldToCamera;
  int u_cameraToView;

  QMatrix4x4 m_projection;
  Camera m_camera;
  Transform m_transform;
  Model *m_model;
};

#endif // WINDOW_H
