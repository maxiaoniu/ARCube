#ifndef WINDOW_H
#define WINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include "backgroud.h"
#include "transform.h"
#include "camera.h"
#include <QMatrix4x4>
#include "model.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QTimer>
class CvCapture;
class GLTexture2D;
class GLTextureCube;
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
  QMatrix4x4 m_backgroundProjection;
  Camera m_camera;
  Transform m_transform;
  Model *m_model;

  //background
  Backgroud *m_back;
  GLTexture2D *m_backTexture;
  GLTextureCube *m_environment;
  QOpenGLShaderProgram *m_backprogram;
  int u_worldToCameraFloor;
  int u_cameraToViewFloor;
  QTimer *m_timer;
  CvCapture* capture;
};

#endif // WINDOW_H
