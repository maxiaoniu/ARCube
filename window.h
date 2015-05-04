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
#include <Leap.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace Leap;
class GLTexture2D;
class GLTextureCube;
class GLRenderTargetCube;
class CubeFace;
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
  //transform
  void generateProjectionMatrix(float height, float width, float far, float near);
  void generateModelViewMatrix();
  // Private Helpers
  void printContextInformation();
  QOpenGLShaderProgram *m_program;

  int u_modelToWorld;
  int u_worldToCamera;
  int u_cameraToView;

  QMatrix4x4 m_projection;
  QMatrix4x4 m_modelview;
  QMatrix4x4 m_backgroundProjection;
  Camera m_camera;
  Transform m_transform;
  Model *m_model;
  Model *m_cubemap;

  //background
  Backgroud *m_back;
  CubeFace *m_cubeface;
  GLTexture2D *m_backTexture;
  GLTextureCube *m_environment;
  QOpenGLShaderProgram *m_backprogram;

  int u_worldToCameraFloor;
  int u_cameraToViewFloor;
  QTimer *m_timer;
  CvCapture* capture;

  //leapmotion
  Controller m_controller;

  //ssbb and texture of cube mapping
  float m_ssbb[4];
  void calcuSSBB(const QMatrix4x4 &m, const QMatrix4x4 &v,const QMatrix4x4 &p);
  QOpenGLShaderProgram *m_cubefaceprogram;
  GLuint texFBO;
  GLuint prepareFBO(int w, int h);
  GLuint m_fbo;
};

#endif // WINDOW_H
