#include "camerasurfaceglitem.h"

#include <QtWidgets>
#include <QtOpenGL>

CameraSurfaceGLItem::CameraSurfaceGLItem(QWidget *parent)
    : QGLWidget(parent)
    ,vid_w(0)
    ,vid_h(0)
    ,win_w(0)
    ,win_h(0)
    ,vao(0)
    ,y_tex(0)
    ,u_tex(0)
    ,v_tex(0)
    ,vert(0)
    ,frag(0)
    ,prog(0)
    ,u_pos(-1)
    ,textures_created(false)
    ,shader_created(false)
{
    m_yuvWidth = 0;
    m_yuvHeight = 0;
}

CameraSurfaceGLItem::~CameraSurfaceGLItem()
{

}


void CameraSurfaceGLItem::slotFrameChanged(QByteArray yuvData, int width, int height)
{
    m_yuvData = yuvData;
    m_yuvWidth = width;
    m_yuvHeight = height;

    update();
}

void CameraSurfaceGLItem::setup_gl(int width, int height)
{
    vid_w = width;
    vid_h = height;

    if(!vid_w || !vid_h) {
      printf("Invalid texture size.\n");
      return;
    }

    if(!setupTextures()) {
      return;
    }

    if(!setupShader()) {
      return;
    }

    glGenVertexArrays(1, &vao);

    return;
}

void CameraSurfaceGLItem::resize_gl(int width, int height)
{
    win_w = width;
    win_h = height;

    pm.setToIdentity();
    pm.ortho(0, win_w, win_h, 0, 0.0, 100.0f);

    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, pm.data());
}

bool CameraSurfaceGLItem::setupTextures()
{
    if(textures_created) {
      printf("Textures already created.\n");
      return false;
    }

    glGenTextures(1, &y_tex);
    glBindTexture(GL_TEXTURE_2D, y_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, vid_w, vid_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &u_tex);
    glBindTexture(GL_TEXTURE_2D, u_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, vid_w/2, vid_h/2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &v_tex);
    glBindTexture(GL_TEXTURE_2D, v_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, vid_w/2, vid_h/2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    textures_created = true;

    return true;
}

bool CameraSurfaceGLItem::setupShader()
{
//    if(shader_created) {
//      printf("Already creatd the shader.\n");
//      return false;
//    }

//    vert = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vert, 1, &YUV420P_VS, 0);
//    glCompileShader(vert);

//    frag = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(frag, 1, &YUV420P_FS, 0);
//    glCompileShader(frag);

//    prog = glCreateProgram();
//    glAttachShader(prog, vert);
//    glAttachShader(prog, frag);
//    glLinkProgram(prog);

//    glUseProgram(prog);
//    glUniform1i(glGetUniformLocation(prog, "y_tex"), 0);
//    glUniform1i(glGetUniformLocation(prog, "u_tex"), 1);
//    glUniform1i(glGetUniformLocation(prog, "v_tex"), 2);

//    u_pos = glGetUniformLocation(prog, "draw_pos");

    return true;
}

void CameraSurfaceGLItem::setYPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, y_tex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w, vid_h, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

void CameraSurfaceGLItem::setUPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, u_tex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w/2, vid_h/2, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

void CameraSurfaceGLItem::setVPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, v_tex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w/2, vid_h/2, GL_RED, GL_UNSIGNED_BYTE, pixels);
}


void CameraSurfaceGLItem::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    setup_gl(1920, 1080);
}


void CameraSurfaceGLItem::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    resize_gl(width, height);
}

void CameraSurfaceGLItem::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(m_yuvData.size())
    {
        setYPixels((unsigned char*)m_yuvData.data(), m_yuvWidth);
        setUPixels((unsigned char*)m_yuvData.data() + m_yuvWidth * m_yuvHeight, m_yuvWidth / 2);
        setVPixels((unsigned char*)m_yuvData.data() + m_yuvWidth * m_yuvHeight * 5 / 4, m_yuvWidth / 2);

        glBindVertexArray(vao);
        glUseProgram(prog);

        QRect frameRect = getFrameRect(rect(), QSize(m_yuvWidth, m_yuvHeight));

        glUniform4f(u_pos, frameRect.left(), frameRect.top(), frameRect.width(), frameRect.height());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, y_tex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, u_tex);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, v_tex);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
