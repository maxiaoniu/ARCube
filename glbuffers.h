/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>

#define BUFFER_OFFSET(i) ((char*)0 + (i))
#define SIZE_OF_MEMBER(cls, member) sizeof(static_cast<cls *>(0)->member)

#define GLBUFFERS_ASSERT_OPENGL(prefix, assertion, returnStatement)                         \
if (m_failed || !(assertion)) {                                                             \
    if (!m_failed) qCritical(prefix ": The necessary OpenGL functions are not available."); \
    m_failed = true;                                                                        \
    returnStatement;                                                                        \
}

QT_BEGIN_NAMESPACE
class QMatrix4x4;
QT_END_NAMESPACE

class GLTexture
{
public:
    GLTexture();
    virtual ~GLTexture();
    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual bool failed() const {return m_failed;}
    GLuint m_texture;
protected:

    bool m_failed;
};

class GLFrameBufferObject
{
public:
    friend class GLRenderTargetCube;
    // friend class GLRenderTarget2D;

    GLFrameBufferObject(int width, int height);
    virtual ~GLFrameBufferObject();
    bool isComplete();
    virtual bool failed() const {return m_failed;}
protected:
    void setAsRenderTarget(bool state = true);
    GLuint m_fbo;
    GLuint m_depthBuffer;
    int m_width, m_height;
    bool m_failed;
};

class GLTexture2D : public GLTexture
{
public:
    GLTexture2D(int width, int height);
    explicit GLTexture2D(const QString& fileName, int width = 0, int height = 0);
    void load(QImage image);
    virtual void bind() Q_DECL_OVERRIDE;
    virtual void unbind() Q_DECL_OVERRIDE;
};

class GLTexture3D : public GLTexture
{
public:
    GLTexture3D(int width, int height, int depth);
    // TODO: Implement function below
    //GLTexture3D(const QString& fileName, int width = 0, int height = 0);
    void load(int width, int height, int depth, QRgb *data);
    virtual void bind() Q_DECL_OVERRIDE;
    virtual void unbind() Q_DECL_OVERRIDE;
};

class GLTextureCube : public GLTexture
{
public:
    GLTextureCube(int size);
    explicit GLTextureCube(const QStringList& fileNames, int size = 0);
    void load(int size, int face, QRgb *data);
    virtual void bind() Q_DECL_OVERRIDE;
    virtual void unbind() Q_DECL_OVERRIDE;
};

// TODO: Define and implement class below
//class GLRenderTarget2D : public GLTexture2D

class GLRenderTargetCube : public GLTextureCube
{
public:
    GLRenderTargetCube(int size);
    // begin rendering to one of the cube's faces. 0 <= face < 6
    void begin(int face);
    // end rendering
    void end();
    virtual bool failed() const Q_DECL_OVERRIDE {return m_failed || m_fbo.failed();}

    static void getViewMatrix(QMatrix4x4& mat, int face);
    static void getProjectionMatrix(QMatrix4x4& mat, float nearZ, float farZ);
private:
    GLFrameBufferObject m_fbo;
};


#endif