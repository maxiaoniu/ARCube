#include <QGuiApplication>
#include "window.h"
#include "modelloader.h"
#include "model.h"
int main(int argc, char *argv[])
{
  QGuiApplication app(argc, argv);

  // Set OpenGL Version information
  // Note: This format must be set before show() is called.
  QSurfaceFormat format;
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(4,1);

  // Set the window up
  Window window;
  window.setFormat(format);
  window.resize(QSize(1280,720));
  window.show();
  return app.exec();
}
