#include "window.h"
#include "modelloader.h"
#include <QDebug>
#include <QString>
#include <QVector4D>
#include "fbm.h"
#include "glbuffers.h"
#include "cubeface.h"
#include <QOpenGLFramebufferObject>

using namespace cv;
Window::Window()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(40);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start();
    //capture = new VideoCapture(0);
    capture = cvCaptureFromCAM( 0 );
    //capture->set(CV_CAP_PROP_FRAME_WIDTH,1280);
    //capture->set(CV_CAP_PROP_FRAME_HEIGHT,720);
}

Window::~Window()
{
  makeCurrent();
  teardownGL();
}

//
/*******************************************************************************
 * Opengl FBO
 ******************************************************************************/

GLuint createRGBATexture(int w, int h) {

    GLuint tex;

    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    for(GLuint i = 0; i < 6; i++)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
        );
    }
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
     glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return(tex);
}

GLuint createBackTexture(int w, int h) {
    GLuint tex;

    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
        GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return(tex);
}
GLuint Window::prepareFBO(int w, int h) {

    GLuint fbo;
    // Generate one frame buffer
    glGenFramebuffers(1, &fbo);
    // bind it
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    // attach textures for colors
    texFBO = createRGBATexture(w,h);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, texFBO, 0);
    // check if everything is OK
    GLenum e = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (e) {

        case GL_FRAMEBUFFER_UNDEFINED:
            printf("FBO Undefined\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
            printf("FBO Incomplete Attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
            printf("FBO Missing Attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER :
            printf("FBO Incomplete Draw Buffer\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED :
            printf("FBO Unsupported\n");
            break;
        case GL_FRAMEBUFFER_COMPLETE:
            printf("FBO OK\n");
            break;
        default:
            printf("FBO Problem?\n");
    }

    if (e != GL_FRAMEBUFFER_COMPLETE)
        return (0);
    // unbind fbo
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

//	printFramebuffersInfo(fbo);

    return fbo;
}

GLuint Window::prepareFBO1(int w, int h) {

    GLuint fbo;
    // Generate one frame buffer
    glGenFramebuffers(1, &fbo);
    // bind it
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    // attach textures for colors
    backFBO = createBackTexture(1280,720);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backFBO, 0);
    // check if everything is OK
    GLenum e = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (e) {

        case GL_FRAMEBUFFER_UNDEFINED:
            printf("FBO Undefined\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
            printf("FBO Incomplete Attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
            printf("FBO Missing Attachment\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER :
            printf("FBO Incomplete Draw Buffer\n");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED :
            printf("FBO Unsupported\n");
            break;
        case GL_FRAMEBUFFER_COMPLETE:
            printf("FBO OK\n");
            break;
        default:
            printf("FBO Problem?\n");
    }

    if (e != GL_FRAMEBUFFER_COMPLETE)
        return (0);
    // unbind fbo
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

//	printFramebuffersInfo(fbo);

    return fbo;
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

    qSort(xList.begin(),xList.end());
    qSort(yList.begin(),yList.end());

    m_ssbb[0]=xList.first()/1280;
    m_ssbb[2]=xList.last()/1280;
    m_ssbb[1]=yList.first()/720;
    m_ssbb[3]=yList.last()/720;
    #define PERCENT_ENLARGE 0.20f
    #define PERCENT_BORDER  0.10f
    float width  = m_ssbb[2] - m_ssbb[0];
    float height = m_ssbb[3] - m_ssbb[1];
    float cx = m_ssbb[0] + ( width / 2.0f );
    float cy = m_ssbb[1] + ( height / 2.0f );
    width *= ( 1.0f + PERCENT_ENLARGE );
    height *= ( 1.0f + PERCENT_ENLARGE );
    m_ssbb[0] = ( cx - ( width / 2.0f ) < PERCENT_BORDER ) ? PERCENT_BORDER : cx - ( width / 2.0f );
    m_ssbb[1] = ( cy - ( height / 2.0f ) < PERCENT_BORDER ) ? PERCENT_BORDER : cy - ( height / 2.0f );
    m_ssbb[2] = ( cx + ( width / 2.0f ) > 1.0f - PERCENT_BORDER ) ? 1.0f - PERCENT_BORDER : cx + ( width / 2.0f );
    m_ssbb[3] = ( cy + ( height / 2.0f ) > 1.0f - PERCENT_BORDER ) ? 1.0f - PERCENT_BORDER : cy + ( height / 2.0f );

    qDebug() <<"x: "<< m_ssbb[0]<<" "<<  m_ssbb[2];
    qDebug() <<"y: "<< m_ssbb[1]<<" "<<  m_ssbb[3];

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
  glDepthFunc(GL_GREATER);
  glEnable(GL_CULL_FACE);

  //create noise tex
  const int NOISE_SIZE = 128; // for a different size, B and BM in fbm.c must also be changed
  m_noise = new GLTexture3D(NOISE_SIZE, NOISE_SIZE, NOISE_SIZE);
  QRgb *data = new QRgb[NOISE_SIZE * NOISE_SIZE * NOISE_SIZE];
  memset(data, 0, NOISE_SIZE * NOISE_SIZE * NOISE_SIZE * sizeof(QRgb));
  QRgb *p = data;
  float pos[3];
  for (int k = 0; k < NOISE_SIZE; ++k) {
      pos[2] = k * (0x20 / (float)NOISE_SIZE);
      for (int j = 0; j < NOISE_SIZE; ++j) {
          for (int i = 0; i < NOISE_SIZE; ++i) {
              for (int byte = 0; byte < 4; ++byte) {
                  pos[0] = (i + (byte & 1) * 16) * (0x20 / (float)NOISE_SIZE);
                  pos[1] = (j + (byte & 2) * 8) * (0x20 / (float)NOISE_SIZE);
                  *p |= (int)(128.0f * (noise3(pos) + 1.0f)) << (byte * 8);
              }
              ++p;
          }
      }
  }
  m_noise->load(NOISE_SIZE, NOISE_SIZE, NOISE_SIZE, data);
  delete[] data;

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
  m_righthandtransform.setToIdentity();
  //m_transform.rotate(20,QVector3D(0,1,0));
  //m_transform.translate(QVector3D(0,152,200));


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
  mLoader.createRoundedBox(0.25,40,10);

  //bunny
  m_bunny = new Model;
  ModelLoader mbLoader(m_bunny);
  //mbLoader.load("bunny.obj");
  //mbLoader.createModel();
  mbLoader.createRoundedBox(0.25,40,10);
  m_righthandprogram = new QOpenGLShaderProgram();
  m_righthandprogram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/bunny.vert");
  m_righthandprogram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/bunny.frag");
  m_righthandprogram->link();
  m_righthandprogram->bind();
  u_modelToWorld = m_program->uniformLocation("modelToWorld");
  u_worldToCamera = m_program->uniformLocation("worldToCamera");
  u_cameraToView = m_program->uniformLocation("cameraToView");
  m_righthandprogram->release();


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

  m_cubefaceprogram = new QOpenGLShaderProgram();
  m_cubefaceprogram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/cube.vert");
  m_cubefaceprogram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/cube.frag");

  m_backprogram->link();
  m_backprogram->bind();

  m_cubefaceprogram->link();
  m_cubefaceprogram->bind();

  u_worldToCameraFloor = m_backprogram->uniformLocation("worldToCamera");
  u_cameraToViewFloor  = m_backprogram->uniformLocation("cameraToView");

  m_backprogram->release();
  m_cubefaceprogram->release();
  m_back = new Backgroud();
  m_back->create();

  m_cubeface = new CubeFace();
  //enviroment mapping
  QStringList list;
  list << ":/shader/cubemap_posx.jpg" << ":/shader/cubemap_negx.jpg" << ":/shader/cubemap_posy.jpg"
       << ":/shader/cubemap_negy.jpg" << ":/shader/cubemap_posz.jpg" << ":/shader/cubemap_negz.png";
  m_environment = new GLTextureCube(list, 1024);

  m_fbo = prepareFBO(512, 512);
  m_backfbo = prepareFBO1(1280, 720);
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
   bool drawflag = true;
   bool drawflag2 = true;
   if( m_controller.isConnected()) //controller is a Controller object
   {
       Frame frame = m_controller.frame(); //The latest frame
       if (!frame.hands().isEmpty())
       {
           Leap::HandList hands = frame.hands();
           Hand firstHand = hands[0];

           Leap::Vector handCenter = firstHand.palmPosition();
           m_transform.setToIdentity();
           //m_transform.rotate(180,QVector3D(0,0,-1));
           m_transform.translate(QVector3D(-handCenter.x,handCenter.y,handCenter.z));
           float yaw = firstHand.direction().yaw();
           m_transform.rotate(-(yaw/3.14)*180,QVector3D(0,-1,0));
           float roll = firstHand.palmNormal().roll();
           m_transform.rotate((roll/3.14)*180,QVector3D(0,0,-1));
           //Frame previous = controller.frame(1); //The previous frame

           if (firstHand.grabStrength()>0.5) {
               drawflag = false;

           }


           Hand secondHand = hands[1];


           Leap::Vector handCenter2 = secondHand.palmPosition();
           m_righthandtransform.setToIdentity();
           //m_transform.rotate(180,QVector3D(0,0,-1));
           m_righthandtransform.translate(QVector3D(-handCenter2.x,handCenter2.y,handCenter2.z));
           float yaw2 = secondHand.direction().yaw();
           m_righthandtransform.rotate(-(yaw2/3.14)*180,QVector3D(0,-1,0));
           float roll2 = secondHand.palmNormal().roll();
           m_righthandtransform.rotate((roll2/3.14)*180,QVector3D(0,0,-1));
           //Frame previous = controller.frame(1); //The previous frame

           if (secondHand.grabStrength()>0.5) {
               drawflag2 = false;


           }
       }
       else
       {
           m_transform.rotate(2,QVector3D(1,1,0));
           m_transform.rotate(2,QVector3D(0,1,0));
           m_transform.setTranslation(QVector3D(40,150,0));
           //m_transform.setScale(QVector3D(5,5,5));
           m_righthandtransform.rotate(2,QVector3D(0,1,0));
           m_righthandtransform.setTranslation(QVector3D(-30,150,0));
          //m_righthandtransform.setScale(QVector3D(250,250,250));
       }

   }
   else
   {
        m_transform.setTranslation(QVector3D(0,150,0));
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
    //generate FBO



    // bind a framebuffer object
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_backfbo);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // Set Drawing buffers
    GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1,  attachments);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,1280,720);
    //glViewport(0,0,1280*2,720*2);
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
    if(drawflag2){
    m_noise->bind();
       //glEnable(GL_TEXTURE_3D);
    m_righthandprogram->bind();
       //m_program->setUniformValue(u_worldToCamera, m_camera.getMatrix());
    m_righthandprogram->setUniformValue(u_worldToCamera, m_modelview);
    m_righthandprogram->setUniformValue(u_cameraToView, m_projection);
    m_righthandprogram->setUniformValue(u_modelToWorld, m_righthandtransform.getMatrix());
    m_righthandprogram->setUniformValue("noise", 0);
    m_bunny->draw();
    m_righthandprogram->release();
    m_noise->unbind();}
       //m_environment->unbind();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLuint attachments1[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1,  attachments1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,512,512);
    //+X
    //m_backTexture->bind();
    //m_backTexture->load(image);
    glBindTexture(GL_TEXTURE_2D, backFBO);
    m_cubefaceprogram->bind();
    m_cubeface->create(
            m_ssbb[2],m_ssbb[3],
            m_ssbb[2] + ( ( 1.0f - m_ssbb[2] ) / 2.0f ) ,m_ssbb[3] + ( ( 1.0f - m_ssbb[3] ) / 2.0f ),
            m_ssbb[2] + ( ( 1.0f - m_ssbb[2] ) / 2.0f ) ,m_ssbb[1] / 2.0f,
            m_ssbb[2],m_ssbb[1]
            );

    m_cubefaceprogram->setUniformValue("screenTexture", 0);
    m_cubeface->draw();
    m_cubefaceprogram->release();
    //m_backTexture->unbind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, texFBO, 0);
    //-X
    //m_backTexture->bind();
    //m_backTexture->load(image);
    glBindTexture(GL_TEXTURE_2D, backFBO);
    m_cubefaceprogram->bind();
    m_cubeface->create(
            m_ssbb[0] / 2.0f,m_ssbb[3] + ( ( 1.0f - m_ssbb[3] ) / 2.0f ),
            m_ssbb[0],m_ssbb[3],
            m_ssbb[0],m_ssbb[1],
            m_ssbb[0]/2.0f,m_ssbb[1] / 2.0f
            );
    m_cubefaceprogram->setUniformValue("screenTexture", 0);
    m_cubeface->draw();
    m_cubefaceprogram->release();
    //m_backTexture->unbind();


//    //+Y
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, texFBO, 0);
    //m_backTexture->bind();
    //m_backTexture->load(image);
    glBindTexture(GL_TEXTURE_2D, backFBO);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    m_cubefaceprogram->bind();
    m_cubeface->create(
            m_ssbb[2] + ( ( 1.0f - m_ssbb[2] ) / 2.0f ),m_ssbb[3] + ( ( 1.0f - m_ssbb[3] ) / 2.0f ),
            m_ssbb[0] / 2.0f,m_ssbb[3] + ( ( 1.0f - m_ssbb[3] ) / 2.0f ),
            m_ssbb[0],m_ssbb[3],
            m_ssbb[2],m_ssbb[3]
            );
    m_cubefaceprogram->setUniformValue("screenTexture", 0);
    m_cubeface->draw();
    m_cubefaceprogram->release();
   // m_backTexture->unbind();
//
//    //-Y
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, texFBO, 0);
    //m_backTexture->bind();
    //m_backTexture->load(image);
    glBindTexture(GL_TEXTURE_2D, backFBO);
    m_cubefaceprogram->bind();
    m_cubeface->create(
                        m_ssbb[2],m_ssbb[1],
                        m_ssbb[0],m_ssbb[1],
                        m_ssbb[0]/2.0f,m_ssbb[1] / 2.0f,
                        m_ssbb[2] + ( ( 1.0f - m_ssbb[2] ) / 2.0f ) ,( m_ssbb[1] / 2.0f )
                        );
    m_cubefaceprogram->setUniformValue("screenTexture", 0);
    m_cubeface->draw();
    m_cubefaceprogram->release();
    m_backTexture->unbind();
    //+Z
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, texFBO, 0);
    //m_backTexture->bind();
    //m_backTexture->load(image);
    glBindTexture(GL_TEXTURE_2D, backFBO);
    m_cubefaceprogram->bind();
    m_cubeface->create( m_ssbb[0],m_ssbb[3],
                        m_ssbb[2],m_ssbb[3],
                        m_ssbb[2],m_ssbb[1],
                        m_ssbb[0],m_ssbb[1]);
    m_cubefaceprogram->setUniformValue("screenTexture", 0);
    m_cubeface->draw();
    m_cubefaceprogram->release();
    //m_backTexture->unbind();
//    //-Z
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, texFBO, 0);
    m_backTexture->bind();
    m_backTexture->load(image);
    glBindTexture(GL_TEXTURE_2D, backFBO);
    m_cubefaceprogram->bind();
    m_cubeface->create(
            m_ssbb[0] / 2.0f,m_ssbb[3] + ( ( 1.0f - m_ssbb[3] ) / 2.0f ),
            m_ssbb[0],m_ssbb[3],
            m_ssbb[0],m_ssbb[1],
            m_ssbb[0]/2.0f,m_ssbb[1] / 2.0f
            );
    m_cubefaceprogram->setUniformValue("screenTexture", 0);
    m_cubeface->draw();
    m_cubefaceprogram->release();
    m_backTexture->unbind();
//
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0,0,1280,720);
    m_backTexture->bind();
    m_backTexture->load(image);
    //glBindTexture(GL_TEXTURE_2D, backFBO);
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

   //m_environment->bind();
   glBindTexture(GL_TEXTURE_CUBE_MAP, texFBO);
   //glEnable(GL_TEXTURE_2D);
   m_program->bind();
   //m_program->setUniformValue(u_worldToCamera, m_camera.getMatrix());
   m_program->setUniformValue(u_worldToCamera, m_modelview);
   m_program->setUniformValue(u_cameraToView, m_projection);
   m_program->setUniformValue(u_modelToWorld, m_transform.getMatrix());
   m_program->setUniformValue("envTex", 0);
   if(drawflag)
        m_model->draw();
   m_program->release();
   //m_environment->unbind();

   m_noise->bind();
   m_righthandprogram->bind();
   m_righthandprogram->setUniformValue(u_worldToCamera, m_modelview);
   m_righthandprogram->setUniformValue(u_cameraToView, m_projection);
   m_righthandprogram->setUniformValue(u_modelToWorld, m_righthandtransform.getMatrix());
   m_righthandprogram->setUniformValue("noise", 0);
   if(drawflag2)
           m_bunny->draw();
   m_righthandprogram->release();
   m_noise->unbind();
      //m_environment->unbind();

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
