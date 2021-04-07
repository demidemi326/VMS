#ifndef CAMERASURFACEGLITEM_H
#define CAMERASURFACEGLITEM_H

#include <QtWidgets>
#include <QGLWidget>
#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>

#include "clientbase.h"
#include "ipcamera.h"


class CameraSurfaceGLItem : public QGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:
    explicit CameraSurfaceGLItem(QWidget *parent = 0);
    ~CameraSurfaceGLItem();

public slots:
    void    slotFrameChanged(QByteArray yuvData, int width, int height);

private:
    void setup_gl(int width, int height);
    void resize_gl(int width, int height);
    bool setupTextures();
    bool setupShader();

    void setYPixels(uint8_t* pixels, int stride);
    void setUPixels(uint8_t* pixels, int stride);
    void setVPixels(uint8_t* pixels, int stride);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
    int vid_w;
    int vid_h;
    int win_w;
    int win_h;
    GLuint vao;
    GLuint y_tex;
    GLuint u_tex;
    GLuint v_tex;
    GLuint vert;
    GLuint frag;
    GLuint prog;
    GLint u_pos;
    bool textures_created;
    bool shader_created;
    QMatrix4x4 pm;

    QByteArray  m_yuvData;
    int         m_yuvWidth;
    int         m_yuvHeight;
};

#endif // CAMERASURFACEGLITEM_H
