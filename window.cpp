#include "window.h"
#include "modelloader.h"
#include <QDebug>
#include <QString>
#include <QVector4D>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "glbuffers.h"


using namespace cv;
Window::Window()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(40);
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
 * Transform Calculate
 ******************************************************************************/
void Window::generateProjectionMatrix(float height, float width, float far, float near)
{
    //camera internal matrix just for my computer
    float Fx = 1.20507632e+03;
    float Fy = 1.22298265e+03;
    float Cx = 6.24119800e+02;
    float Cy = 4.22267512e+02;

    QVector4D col01 = QVector4D(2*Fx/width,0,0,0);
    QVector4D col02 = QVector4D(0,2*Fy/height,0,0);
    QVector4D col03 = QVector4D(1-2*Cx/width,
                                2*Cy/height-1,
                                -(far+near)/(far-near),
                                -1);
    QVector4D col04 = QVector4D(0,0,-(2*far*near)/(far-near),0);

    m_projection.setColumn(0,col01);
    m_projection.setColumn(1,col02);
    m_projection.setColumn(2,col03);
    m_projection.setColumn(3,col04);
    qDebug()<<m_projection;

}
void Window::generateModelViewMatrix()
{
    typedef double precision;
    double r[3][1]={0.025563, 0.21087334,-3.12837409};
    double t[3][1]={11.29395024, 158.78401244,287.38354452};
    Mat rvec(3,1,CV_64FC1, r);
    Mat tvec(3,1,CV_64FC1, t);

    Mat rotation;
    Rodrigues(rvec, rotation);
    double offsetA[3][1] = {9,6,6};
    Mat offset(3, 1, CV_64FC1, offsetA);
    Mat translation;
    translation = tvec+ rotation*offset;
    Mat modelview(4,4,CV_64FC1);
    modelview.at<precision>(3,0) = 0;
    modelview.at<precision>(0,0) = rotation.at<precision>(0,0);
    modelview.at<precision>(1,0) = rotation.at<precision>(1,0);
    modelview.at<precision>(2,0) = rotation.at<precision>(2,0);
    modelview.at<precision>(3,0) = 0;

    modelview.at<precision>(0,1) = rotation.at<precision>(0,1);
    modelview.at<precision>(1,1) = rotation.at<precision>(1,1);
    modelview.at<precision>(2,1) = rotation.at<precision>(2,1);
    modelview.at<precision>(3,1) = 0;

    modelview.at<precision>(0,2) = rotation.at<precision>(0,2);
    modelview.at<precision>(1,2) = rotation.at<precision>(1,2);
    modelview.at<precision>(2,2) = rotation.at<precision>(2,2);
    modelview.at<precision>(3,2) = 0;

    modelview.at<precision>(0,3) = translation.at<precision>(0,0);
    modelview.at<precision>(1,3) = translation.at<precision>(1,0);
    modelview.at<precision>(2,3) = translation.at<precision>(2,0);
    modelview.at<precision>(3,3) = 1;

    // This matrix corresponds to the change of coordinate systems.
    static double changeCoordArray[4][4] = {{1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, 1}};
    static Mat changeCoord(4, 4, CV_64FC1, changeCoordArray);

    modelview = changeCoord*modelview;

    QVector4D col01 = QVector4D(modelview.at<precision>(0,0),
                                modelview.at<precision>(1,0),
                                modelview.at<precision>(2,0),
                                modelview.at<precision>(3,0)
                                );
    QVector4D col02 = QVector4D(modelview.at<precision>(0,1),
                                modelview.at<precision>(1,1),
                                modelview.at<precision>(2,1),
                                modelview.at<precision>(3,1)
                                );
    QVector4D col03 = QVector4D(modelview.at<precision>(0,2),
                                modelview.at<precision>(1,2),
                                modelview.at<precision>(2,2),
                                modelview.at<precision>(3,2)
                                );
    QVector4D col04 = QVector4D(modelview.at<precision>(0,3),
                                modelview.at<precision>(1,3),
                                modelview.at<precision>(2,3),
                                modelview.at<precision>(3,3)
                                );
    m_modelview.setColumn(0,col01);
    m_modelview.setColumn(1,col02);
    m_modelview.setColumn(2,col03);
    m_modelview.setColumn(3,col04);
    qDebug()<<m_modelview;

}

//calcualte the screen space bounding box
void Window::calcuSSBB(const QMatrix4x4 &m, const QMatrix4x4 &v,const QMatrix4x4 &p)
{
    QVector4D res;
    QMatrix4x4 mvp = p*v*m;
    QList<float> xList;
    QList<float> yList;

    //FTR
    QVector4D ftr =QVector4D(21,21,21,1);
    res = mvp*ftr;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);
    //FTL
    QVector4D ftl =QVector4D(-21,21,21,1);
    res = mvp*ftl;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);
    //FBR
    QVector4D fbr =QVector4D(21,-21,21,1);
    res = mvp*fbr;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);
    //FBL
    QVector4D fbl =QVector4D(-21,-21,21,1);
    res = mvp*fbl;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);

    //BTR
    QVector4D btr =QVector4D(21,21,-21,1);
    res = mvp*btr;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);
    //BTL
    QVector4D btl =QVector4D(-21,21,-21,1);
    res = mvp*btl;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);
    //BBR
    QVector4D bbr =QVector4D(21,-21,-21,1);
    res = mvp*bbr;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);
    //BBL
    QVector4D bbl =QVector4D(-21,-21,-21,1);
    res = mvp*bbl;
    xList.append((res.x()/res.z()+1)*0.5*1279);
    yList.append((1-(res.y()/res.z()+1)*0.5)*719);

    qSort(xList);
    qSort(yList);

    m_ssbb[0]=xList.first();
    m_ssbb[2]=xList.last();
    m_ssbb[1]=yList.first();
    m_ssbb[3]=yList.last();

    //qDebug() <<"x: "<<xList.first()<<" "<< xList.last();
    //qDebug() <<"y: "<<yList.first()<<" "<< yList.last();


}
//calculate the uv of cube texture
//void Window::createCubeUV()
//{


//}

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
  //m_projection.perspective(60.0f, width() / float(height()), 0.1f, 3000.0f);
  generateProjectionMatrix(720.0,1280.0,3000.0,1.0);
  m_modelview.setToIdentity();
  generateModelViewMatrix();
  m_backgroundProjection.setToIdentity();
  m_backgroundProjection.ortho(0, 1280, 0, 720, 19, 21);
  m_camera.setToIdentity();
  m_camera.translate(0,0,-5);

  m_transform.setToIdentity();
  //m_transform.rotate(20,QVector3D(0,1,0));
  m_transform.translate(QVector3D(0,152,200));


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
  mLoader.createRoundedBox(0.25,40,10);

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
       << ":/shader/cubemap_negy.jpg" << ":/shader/cubemap_posz.jpg" << ":/shader/back.png";
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
   //LeapMotion
   if( m_controller.isConnected()) //controller is a Controller object
   {
       Frame frame = m_controller.frame(); //The latest frame
       Leap::HandList hands = frame.hands();
       Hand firstHand = hands[0];
       Leap::Vector handCenter = firstHand.palmPosition();
       m_transform.setTranslation(QVector3D(-handCenter.x,handCenter.y,handCenter.z));
       //float yaw = firstHand.direction().yaw();
       //m_transform.setRotation(-(yaw/3.14)*180,0,-1,0);
       //float roll = firstHand.palmNormal().roll();
       //m_transform.setRotation((roll/3.14)*180,0,0,-1);
       //Frame previous = controller.frame(1); //The previous frame
   }
   //create dynamic enviroment mapping
   calcuSSBB(m_transform.getMatrix(),m_modelview,m_projection);
   //opencv
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
   //m_program->setUniformValue(u_worldToCamera, m_camera.getMatrix());
   m_program->setUniformValue(u_worldToCamera, m_modelview);
   m_program->setUniformValue(u_cameraToView, m_projection);
   m_program->setUniformValue(u_modelToWorld, m_transform.getMatrix());
   m_program->setUniformValue("envTex", 0);

   m_model->draw();
   //m_box->draw();
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
