#include "window.h"
#include "modelloader.h"
#include <QDebug>
#include <QString>
Window::Window()
{


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
  printContextInformation();

  // Set global information
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


  m_projection.setToIdentity();
  m_projection.perspective(60.0f, width() / float(height()), 0.1f, 3000.0f);

  m_camera.setToIdentity();
  m_camera.setTranslation(0,0,-5);

  m_transform.setToIdentity();

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
  glClear(GL_COLOR_BUFFER_BIT);

   m_program->bind();

   m_program->setUniformValue(u_worldToCamera, m_camera.getMatrix());
   m_program->setUniformValue(u_cameraToView, m_projection);
   m_program->setUniformValue(u_modelToWorld, m_transform.getMatrix());

   m_model->draw();
   m_program->release();
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
