#include "window.h"
#include "modelloader.h"
#include <QDebug>
#include <QString>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "glbuffers.h"
using namespace cv;
Window::Window()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(30);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start();

    capture = cvCaptureFromCAM( 0 );


}

Window::~Window()
{
  makeCurrent();
  teardownGL();
}

/*******************************************************************************
 * OpenGL Events
 ******************************************************************************/

void Window::initializeGL()
{
  // Initialize OpenGL Backend
  initializeOpenGLFunctions();
//  printContextInformation();

  // Set global information
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);


  m_projection.setToIdentity();
  m_projection.perspective(60.0f, width() / float(height()), 0.1f, 3000.0f);
  m_backgroundProjection.setToIdentity();
  m_backgroundProjection.ortho(0, 1280, 0, 720, 19, 21);
  m_camera.setToIdentity();
  m_camera.translate(0,0,-5);

  m_transform.setToIdentity();
  //m_transform.rotate(45,QVector3D(0,1,0));
  m_transform.scale(2);

  m_program = new QOpenGLShaderProgram();
  m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/simple.vert");
  m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/simple.frag");

  m_program->link();
  m_program->bind();

  u_modelToWorld = m_program->uniformLocation("modelToWorld");
  u_worldToCamera = m_program->uniformLocation("worldToCamera");
  u_cameraToView = m_program->uniformLocation("cameraToView");

  m_program->release();

  m_model = new Model;
  ModelLoader mLoader(m_model);

  mLoader.load("cube.obj");
  mLoader.createModel();

  //draw background
  //m_backTexture = new QOpenGLTexture(QImage(":/shader/back.png"),QOpenGLTexture::DontGenerateMipMaps);
  //m_backTexture->setWrapMode(QOpenGLTexture::Repeat);
  //m_backTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  //m_backTexture->setMagnificationFilter(QOpenGLTexture::Linear);
  //m_backTexture->allocateStorage();
  m_backTexture = new GLTexture2D(1280,720);
  m_backprogram = new QOpenGLShaderProgram();
  m_backprogram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/backgroud.vert");
  m_backprogram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/backgroud.frag");

  m_backprogram->link();
  m_backprogram->bind();

  u_worldToCameraFloor = m_backprogram->uniformLocation("worldToCamera");
  u_cameraToViewFloor  = m_backprogram->uniformLocation("cameraToView");

  m_backprogram->release();
  m_back = new Backgroud();
  m_back->create();
  //enviroment mapping
  QStringList list;
  list << ":/shader/cubemap_posx.jpg" << ":/shader/cubemap_negx.jpg" << ":/shader/cubemap_posy.jpg"
       << ":/shader/cubemap_negy.jpg" << ":/shader/cubemap_posz.jpg" << ":/shader/cubemap_negz.jpg";
  m_environment = new GLTextureCube(list, 1024);

}

void Window::resizeGL(int width, int height)
{
  // Currently we are not handling width/height changes
  (void)width;
  (void)height;
}

void Window::paintGL()
{
  // Clear
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   Mat opencv_image = cvQueryFrame(capture);
   Mat dest;
   cvtColor(opencv_image, dest,CV_BGR2RGB);
   QImage image((uchar*)dest.data, dest.cols, dest.rows,QImage::Format_RGB888);

  //m_backTexture->bind(0);
   //m_backTexture->setData(image,QOpenGLTexture::DontGenerateMipMaps);

   //glActiveTexture(GL_TEXTURE0);
   m_backTexture->bind();
   m_backTexture->load(image);
   m_backprogram->bind();

   m_backprogram->setUniformValue("ourTexture", 0);
   m_backprogram->setUniformValue(u_worldToCameraFloor, m_camera.getMatrix());
   m_backprogram->setUniformValue(u_cameraToViewFloor, m_backgroundProjection);
   if(m_back!= NULL)
   {
       m_back->draw();
   }
   m_backprogram->release();
   m_backTexture->unbind();

   m_environment->bind();
   m_program->bind();
   m_program->setUniformValue(u_worldToCamera, m_camera.getMatrix());
   m_program->setUniformValue(u_cameraToView, m_projection);
   m_program->setUniformValue(u_modelToWorld, m_transform.getMatrix());
   m_program->setUniformValue("envTex", 0);
   m_model->draw();
   m_program->release();
   m_environment->unbind();

}

void Window::teardownGL()
{
  // Currently we have no data to teardown
}

/*******************************************************************************
 * Private Helpers
 ******************************************************************************/

void Window::printContextInformation()
{
  QString glType;
  QString glVersion;
  QString glProfile;

  // Get Version Information
  glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

  // Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
  switch (format().profile())
  {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
  }
#undef CASE

  // qPrintable() will print our QString w/o quotes around it.
  qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}
