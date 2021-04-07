#include "camerasurfaceglview.h"
#include "clientbase.h"
#include "facerequestsocket.h"
#include "serverinfosocket.h"
#include "stringtable.h"
#include "ipcamera.h"
#include "controlbutton.h"

#include <QtWidgets>

extern void convertYUV420P_toRGB888(unsigned char* data, int width, int height, unsigned char* dstData);
extern void convertYUV420P_toRGB888_Scale4(unsigned char* data, int width, int height, unsigned char* dstData);

void frameReceiveProc(void* param);

#ifdef USE_GL

const char* YUV420P_VS = ""
  "#version 330\n"
  ""
  "uniform mat4 u_pm;"
  "uniform vec4 draw_pos;"
  ""
  "const vec2 verts[4] = vec2[] ("
  "  vec2(-0.5,  0.5), "
  "  vec2(-0.5, -0.5), "
  "  vec2( 0.5,  0.5), "
  "  vec2( 0.5, -0.5)  "
  ");"
  ""
  "const vec2 texcoords[4] = vec2[] ("
  "  vec2(0.0, 1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)  "
  "); "
  ""
  "out vec2 v_coord; "
  ""
  "void main() {"
  "   vec2 vert = verts[gl_VertexID];"
  "   vec4 p = vec4((0.5 * draw_pos.z) + draw_pos.x + (vert.x * draw_pos.z), "
  "                 (0.5 * draw_pos.w) + draw_pos.y + (vert.y * draw_pos.w), "
  "                 0, 1);"
  "   gl_Position = u_pm * p;"
  "   v_coord = texcoords[gl_VertexID];"
  "}"
  "";

//"const vec3 R_cf = vec3(1,  0.000000,  1.28033);"
//"const vec3 G_cf = vec3(1, -0.21482, -0.382968);"
//"const vec3 B_cf = vec3(1,  2.12798,  0.000000);"

//"const vec3 R_cf = vec3(1,  0.000000,  1.13983);"
//"const vec3 G_cf = vec3(1, -0.39465, -0.58060);"
//"const vec3 B_cf = vec3(1,  2.03211,  0.000000);"

//"const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
//"const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
//"const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"

const char* YUV420P_FS = ""
 "#version 330\n"
  "uniform sampler2D y_tex;"
  "uniform sampler2D u_tex;"
  "uniform sampler2D v_tex;"
  "in vec2 v_coord;"
  "layout( location = 0 ) out vec4 fragcolor;"
  ""
    "const vec3 R_cf = vec3(1,  0.000000,  1.13983);"
    "const vec3 G_cf = vec3(1, -0.39465, -0.58060);"
    "const vec3 B_cf = vec3(1,  2.03211,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
  ""
  "void main() {"
  "  float y = texture(y_tex, v_coord).r;"
  "  float u = texture(u_tex, v_coord).r;"
  "  float v = texture(v_tex, v_coord).r;"
  "  vec3 yuv = vec3(y,u,v);"
  "  yuv += offset;"
  "  fragcolor = vec4(0.0, 0.0, 0.0, 1.0);"
  "  fragcolor.r = dot(yuv, R_cf);"
  "  fragcolor.g = dot(yuv, G_cf);"
  "  fragcolor.b = dot(yuv, B_cf);"
  "}"
  "";

CameraSurfaceGLView::CameraSurfaceGLView(QWidget *parent) : QOpenGLWidget(parent)
  ,vid_w(0)
  ,vid_h(0)
  ,win_w(0)
  ,win_h(0)
  ,y_tex(0)
  ,u_tex(0)
  ,v_tex(0)
  ,vert(0)
  ,frag(0)
  ,prog(0)
  ,u_pos(-1)
{
    m_frameWidth = 1;
    m_frameHeight = 1;
    m_chanelIndex = 0;
    m_mediaIndex = 0;
    m_selected = 0;
    m_initialized = 0;

    m_faceRequestSocket = NULL;
    m_serverInfoSocket = NULL;

    m_closeNormalPix = QPixmap(":/images/camera_close_normal.png");
    m_closeUpPix = QPixmap(":/images/camera_close_up.png");
    m_closeDownPix = QPixmap(":/images/camera_close_down.png");

    m_maximumNormalPix = QPixmap(":/images/camera_maximum_normal.png");
    m_maximumUpPix = QPixmap(":/images/camera_maximum_up.png");
    m_maximumDownPix = QPixmap(":/images/camera_maximum_down.png");

    m_tileNormalPix = QPixmap(":/images/camera_tile_normal.png");
    m_tileUpPix = QPixmap(":/images/camera_tile_up.png");
    m_tileDownPix = QPixmap(":/images/camera_tile_down.png");

    m_cameraNonePix = QPixmap(":/images/camera_none.png");

    m_isMaximumScreen = 0;
    m_create = 0;

    m_chanelStatus = CHANEL_STATUS_STOP;

    m_maximumButton = new ControlButton(this);
    m_closeButton = new ControlButton(this);

    m_maximumButton->setButtonPix(m_maximumNormalPix, m_maximumUpPix, m_maximumDownPix);
    m_closeButton->setButtonPix(m_closeNormalPix, m_closeUpPix, m_closeDownPix);

    m_ipCamera = new IpCamera;
    m_ipCamera->setReconnect(0);
    qRegisterMetaType<YUVFrame>("YUVFrame");

    connect(m_ipCamera, SIGNAL(frameChanged(YUVFrame)), this, SLOT(slotFrameChanged(YUVFrame)));
    connect(m_maximumButton, SIGNAL(clicked()), this, SLOT(slotMaximumClicked()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));
}

CameraSurfaceGLView::~CameraSurfaceGLView()
{

}

void CameraSurfaceGLView::startMonitoring(ServerInfoSocket* serverInfoSocket, int chanelIndex, QString monitoringAreaName, int mediaIndex)
{
    stopMonitoring();

    m_monitoringAreaName = monitoringAreaName;
    m_serverInfoSocket = serverInfoSocket;
    m_chanelIndex = chanelIndex;
    m_mediaIndex = mediaIndex;

    m_chanelStatus = m_serverInfoSocket->getChanelStatus(chanelIndex);
    connect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int, int, int)));

    if(m_chanelStatus != CHANEL_STATUS_STOP)
        start();

    m_maximumButton->show();
    m_closeButton->show();
}

void CameraSurfaceGLView::stopMonitoring()
{
    stop();

    if(m_serverInfoSocket)
    {
        disconnect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int, int, int)));
        m_serverInfoSocket = NULL;
    }

    m_monitoringAreaName = "";

    m_maximumButton->hide();
    m_closeButton->hide();

}

void CameraSurfaceGLView::start()
{
    if(m_serverInfoSocket == NULL)
        return;

    if(m_faceRequestSocket)
        stop();

    m_faceRequestSocket = new FaceRectReceiveSocket;
    m_faceRequestSocket->setInfo(m_serverInfoSocket, m_chanelIndex);
    m_faceRequestSocket->slotReconnect();

    connect(m_faceRequestSocket, SIGNAL(receivedFaceResult(QVector<QRect>, qint64, qint64)), this, SLOT(slotFaceResults(QVector<QRect>, qint64, qint64)));

    m_ipCamera->open(m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].ipAddress, m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].userName,
                     m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].password);
}

void CameraSurfaceGLView::stop()
{
    if(m_faceRequestSocket)
    {
        m_faceRequestSocket->stopSocket();
        delete m_faceRequestSocket;
        m_faceRequestSocket = NULL;
    }

    m_ipCamera->close();

    releaseGL();
    setEmpty();
}

ServerInfoSocket* CameraSurfaceGLView::serverInfoSocket()
{
    return m_serverInfoSocket;
}

int CameraSurfaceGLView::chanelIndex()
{
    return m_chanelIndex;
}

int CameraSurfaceGLView::isRunning()
{
    return m_ipCamera->isRunning();
}

void CameraSurfaceGLView::setMaximum(int maximum)
{
    m_isMaximumScreen = maximum;
    if(m_isMaximumScreen == 0)
    {
        m_maximumButton->setButtonPix(m_maximumNormalPix, m_maximumUpPix, m_maximumDownPix);
    }
    else
    {
        m_maximumButton->setButtonPix(m_tileNormalPix, m_tileUpPix, m_tileDownPix);
    }
    update();
}

int CameraSurfaceGLView::isMaximum()
{
    return m_isMaximumScreen;
}

void CameraSurfaceGLView::setSelected(int select)
{
    m_selected = select;
    update();
}

int CameraSurfaceGLView::isSelected()
{
    return m_selected;
}

QImage CameraSurfaceGLView::currentImage()
{
    if(m_stackFrameData.size() == 0)
        return QImage();

    QByteArray yuvData = m_stackFrameData[m_stackFrameData.size() - 1];

    QImage currentImage(m_frameWidth, m_frameHeight, QImage::Format_RGB888);
    convertYUV420P_toRGB888((unsigned char*)yuvData.data(), m_frameWidth, m_frameHeight, currentImage.bits());

    return currentImage;
}

void CameraSurfaceGLView::slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus)
{
    if(m_chanelIndex != chanelIndex)
        return;

    if(m_chanelStatus == chanelStatus)
        return;

    m_chanelStatus = chanelStatus;
    if(m_chanelStatus == CHANEL_STATUS_STOP)
        stop();
    else
        start();
}

void CameraSurfaceGLView::slotFaceResults(QVector<QRect> faceResults, qint64 serverTime, qint64 diffTime)
{
    qint64 curTime = serverTime - diffTime;
    if(m_stackTime.size() == 0)
    {
        setEmpty();
        return;
    }
    int index = 0;
    for(int i = 0; i < m_stackTime.size() - 1; i ++)
    {
        if(curTime >= m_stackTime[i] && curTime <= m_stackTime[i + 1])
        {
            index = i;
            if(m_stackTime[i + 1] - curTime < curTime - m_stackTime[i + 1])
                index = i + 1;

            break;
        }
    }

    if(curTime < m_stackTime[0])
    {
        index = 0;
    }
    else if(curTime > m_stackTime[m_stackTime.size() - 1])
    {
        index = m_stackTime.size() - 1;
    }

    m_frameImageData = m_stackFrameData[index];
    m_faceResults = faceResults;

    update();
}

void CameraSurfaceGLView::slotFrameChanged(YUVFrame yuvFrame)
{
    if(m_initialized == 0)
        return;

    if(yuvFrame.yuvData == 0 || !m_ipCamera->isRunning())
    {
        m_frameImageData.clear();
        update();
        return;
    }

    if(m_create == 0)
    {
        createGL(yuvFrame.width, yuvFrame.height);
        resizeGL();

        m_create = 1;
    }

#define MAX_STACK_SIZE 15
    QByteArray yuvData((char*)yuvFrame.yuvData, yuvFrame.width * yuvFrame.height * 3 / 2);
    m_frameWidth = yuvFrame.width;
    m_frameHeight = yuvFrame.height;

    m_stackFrameData.append(yuvData);
    m_stackTime.append(yuvFrame.frameTime);
    if(m_stackFrameData.size() > MAX_STACK_SIZE)
    {
        m_stackFrameData.remove(0, m_stackFrameData.size() - MAX_STACK_SIZE);
        m_stackTime.remove(0, m_stackTime.size() - MAX_STACK_SIZE);
    }
}


void CameraSurfaceGLView::setEmpty()
{
    m_stackFrameData.clear();
    m_stackTime.resize(0);
    m_faceResults.resize(0);
    m_frameImageData.resize(0);

    update();
}

void CameraSurfaceGLView::mousePressEvent(QMouseEvent* e)
{
    QOpenGLWidget::mousePressEvent(e);

    emit selected(this);
    update();
}



void CameraSurfaceGLView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QOpenGLWidget::mouseDoubleClickEvent(e);

    if(m_serverInfoSocket)
    {
        if(isMaximum())
            setMaximum(0);
        else
            setMaximum(1);

        emit maximumChanged();
    }
}

void CameraSurfaceGLView::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    m_initialized = 1;
}

void CameraSurfaceGLView::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    if(m_create)
        resizeGL();

    QRect closeRect = QRect(rect().right() - 20, rect().top() + 7, m_closeNormalPix.width(), m_closeNormalPix.height());
    QRect maximumRect = QRect(rect().right() - 33, rect().top() + 7, m_maximumNormalPix.width(), m_maximumNormalPix.height());

    m_maximumButton->setGeometry(maximumRect);
    m_closeButton->setGeometry(closeRect);
}


void CameraSurfaceGLView::paintEvent(QPaintEvent *e)
{
    if(!isVisible())
        return;

    int yuvDrawFlag = 0;
    QPainter painter;
    painter.begin(this);
    painter.beginNativePainting();

    if(m_frameImageData.size() && m_chanelStatus != CHANEL_STATUS_STOP && m_ipCamera->isRunning() && m_create)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        setYPixels((unsigned char*)m_frameImageData.data(), m_frameWidth);
        setUPixels((unsigned char*)m_frameImageData.data() + m_frameWidth * m_frameHeight, m_frameWidth / 2);
        setVPixels((unsigned char*)m_frameImageData.data() + m_frameWidth * m_frameHeight * 5 / 4, m_frameWidth / 2);

        glEnable(GL_TEXTURE_2D);
        glUseProgram(prog);

        QRect frameRect = getFrameRect(rect(), QSize(m_frameWidth, m_frameHeight));

        glUniform4f(u_pos, frameRect.left(), frameRect.top(), frameRect.width(), frameRect.height());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, y_tex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, u_tex);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, v_tex);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glUseProgram(0);

        yuvDrawFlag = 1;
    }
    painter.endNativePainting();


    if(yuvDrawFlag)
    {
        QVector<QRect> faceResults = m_faceResults;
        QRect frameRect = getFrameRect(rect(), QSize(m_frameWidth, m_frameHeight));

        for(int i = 0; i < faceResults.size(); i ++)
        {
            QRect faceRect = faceResults[i];

#ifndef _IMAGE_HALF_
            faceRect = QRect(frameRect.left() + faceRect.left() * frameRect.width() / (float)m_frameWidth,
                             frameRect.top() + faceRect.top() * frameRect.height() / (float)m_frameHeight,
                             faceRect.width() * frameRect.width() / (float)m_frameWidth,
                             faceRect.height() * frameRect.height() / (float)m_frameHeight);
#else
            faceRect = QRect(frameRect.left() + (faceRect.left() / 2) * frameRect.width() / (float)frameImage.width(),
                             frameRect.top() + (faceRect.top() / 2) * frameRect.height() / (float)frameImage.height(),
                             faceRect.width() * (frameRect.width() / 2) / (float)frameImage.width(),
                             faceRect.height() * (frameRect.height() / 2) / (float)frameImage.height());
#endif

            painter.save();
            painter.setPen(QPen(QColor(Qt::green), 1));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawRect(faceRect);

            painter.setBrush(QColor(Qt::green));

            qreal rounded = (qreal)faceRect.width() / 80;
            QRect borderRect(faceRect.left(), faceRect.top() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 4, faceRect.top() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left(), faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 4, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.top(), faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 4, faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.top(), faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 4, faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.top() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.top() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            painter.restore();

        }

        painter.save();
        if(isSelected())
            painter.setPen(QPen(QColor(0, 255, 0), 2));
        else
            painter.setPen(QPen(QColor(100, 100, 100), 2));
        QRect boundingRect(rect().left() + 1, rect().top() + 1, rect().width() - 2, rect().height() - 2);
        painter.drawRect(boundingRect);

        painter.restore();
    }
    else
    {
        painter.save();
        painter.drawPixmap(rect(), m_cameraNonePix);
        painter.restore();

        painter.save();
        if(isSelected())
            painter.setPen(QPen(QColor(0, 255, 0), 2));
        else
            painter.setPen(QPen(QColor(100, 100, 100), 2));
        QRect boundingRect(rect().left() + 1, rect().top() + 1, rect().width() - 2, rect().height() - 2);
        painter.drawRect(boundingRect);

        painter.restore();
    }

    if(m_serverInfoSocket)
    {
        ServerInfo serverInfo = m_serverInfoSocket->serverInfo();

        painter.save();

        QString cameraStr;
        cameraStr.sprintf("%s - %s (%s %d)", m_monitoringAreaName.toUtf8().data(), serverInfo.serverName.toUtf8().data(),
                          StringTable::Str_Chanel.toUtf8().data(), m_chanelIndex + 1);
        QRect cameraStrRect(rect().left() + 10, rect().top() + 10, rect().width() - 10, 30);

        QFont cameraFont = painter.font();
        cameraFont.setPointSize(10);

        painter.setFont(cameraFont);
        painter.setPen(Qt::red);
        painter.drawText(cameraStrRect, Qt::AlignLeft | Qt::AlignVCenter, cameraStr);

        painter.restore();

        if(m_chanelStatus == CHANEL_STATUS_STOP)
        {
            painter.save();

            QFont cameraFont = painter.font();
            cameraFont.setPointSize(14);

            painter.setFont(cameraFont);
            painter.setPen(Qt::red);
            painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, StringTable::Str_Disconnected);

            painter.restore();
        }
    }

    painter.end();
}

void CameraSurfaceGLView::createGL(int width, int height)
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
}

void CameraSurfaceGLView::releaseGL()
{
    if(m_create)
    {
        makeCurrent();
        glDetachShader(prog, vert);
        glDetachShader(prog, vert);

        glDeleteProgram(prog);

        glDeleteTextures(1, &y_tex);
        glDeleteTextures(1, &u_tex);
        glDeleteTextures(1, &v_tex);
        doneCurrent();

        m_create = 0;
    }
}

void CameraSurfaceGLView::resizeGL()
{
    win_w = rect().width();
    win_h = rect().height();

    pm.setToIdentity();
    pm.ortho(0, win_w, win_h, 0, 0.0, 100.0f);

    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_pm"), 1, GL_FALSE, pm.data());
}


bool CameraSurfaceGLView::setupTextures()
{
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

    return true;
}

bool CameraSurfaceGLView::setupShader()
{
    vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &YUV420P_VS, 0);
    glCompileShader(vert);

    frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &YUV420P_FS, 0);
    glCompileShader(frag);

    prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    glUseProgram(prog);
    glUniform1i(glGetUniformLocation(prog, "y_tex"), 0);
    glUniform1i(glGetUniformLocation(prog, "u_tex"), 1);
    glUniform1i(glGetUniformLocation(prog, "v_tex"), 2);

    u_pos = glGetUniformLocation(prog, "draw_pos");

    glUseProgram(0);

    return true;
}

void CameraSurfaceGLView::setYPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, y_tex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w, vid_h, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

void CameraSurfaceGLView::setUPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, u_tex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w/2, vid_h/2, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

void CameraSurfaceGLView::setVPixels(uint8_t* pixels, int stride)
{
    glBindTexture(GL_TEXTURE_2D, v_tex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vid_w/2, vid_h/2, GL_RED, GL_UNSIGNED_BYTE, pixels);
}

void  CameraSurfaceGLView::slotMaximumClicked()
{
    if(m_serverInfoSocket)
    {
        if(isMaximum())
            setMaximum(0);
        else
            setMaximum(1);

        emit maximumChanged();
    }
}

void CameraSurfaceGLView::slotCloseClicked()
{
    stopMonitoring();
    emit closeClicked(this);

    if(isMaximum())
    {
        setMaximum(0);
        emit maximumChanged();
    }
}

#else

#ifdef SEND_SCORE
extern float g_faceScore;
#endif

CameraSurfaceGLView::CameraSurfaceGLView(QWidget *parent) : QWidget(parent)
{
    m_frameWidth = 1;
    m_frameHeight = 1;
    m_chanelIndex = 0;
    m_mediaIndex = 0;
    m_selected = 0;

    m_faceRequestSocket = NULL;
    m_serverInfoSocket = NULL;

    m_closeNormalPix = QPixmap(":/images/camera_close_normal.png");
    m_closeUpPix = QPixmap(":/images/camera_close_up.png");
    m_closeDownPix = QPixmap(":/images/camera_close_down.png");

    m_maximumNormalPix = QPixmap(":/images/camera_maximum_normal.png");
    m_maximumUpPix = QPixmap(":/images/camera_maximum_up.png");
    m_maximumDownPix = QPixmap(":/images/camera_maximum_down.png");

    m_tileNormalPix = QPixmap(":/images/camera_tile_normal.png");
    m_tileUpPix = QPixmap(":/images/camera_tile_up.png");
    m_tileDownPix = QPixmap(":/images/camera_tile_down.png");

    m_cameraNonePix = QPixmap(":/images/camera_none.png");

    m_isMaximumScreen = 0;

    m_chanelStatus = CHANEL_STATUS_STOP;

    m_maximumButton = new ControlButton(this);
    m_closeButton = new ControlButton(this);

    m_maximumButton->setButtonPix(m_maximumNormalPix, m_maximumUpPix, m_maximumDownPix);
    m_closeButton->setButtonPix(m_closeNormalPix, m_closeUpPix, m_closeDownPix);

    m_ipCamera = new IpCamera;
    qRegisterMetaType<YUVFrame>("YUVFrame");

    connect(m_ipCamera, SIGNAL(frameChanged()), this, SLOT(slotFrameChanged()));
    connect(m_maximumButton, SIGNAL(clicked()), this, SLOT(slotMaximumClicked()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));
}

CameraSurfaceGLView::~CameraSurfaceGLView()
{

}

void CameraSurfaceGLView::startMonitoring(ServerInfoSocket* serverInfoSocket, int chanelIndex, QString monitoringAreaName, int mediaIndex)
{
    stopMonitoring();

    m_monitoringAreaName = monitoringAreaName;
    m_serverInfoSocket = serverInfoSocket;
    m_chanelIndex = chanelIndex;
    m_mediaIndex = mediaIndex;

    m_chanelStatus = m_serverInfoSocket->getChanelStatus(chanelIndex);
    connect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int, int, int)));

    if(m_chanelStatus != CHANEL_STATUS_STOP)
        start();

    m_maximumButton->show();
    m_closeButton->show();
}

void CameraSurfaceGLView::stopMonitoring()
{
    stop();

    if(m_serverInfoSocket)
    {
        disconnect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int, int, int)));
        m_serverInfoSocket = NULL;
    }

    m_monitoringAreaName = "";

    m_maximumButton->hide();
    m_closeButton->hide();

}

void CameraSurfaceGLView::start()
{
    if(m_serverInfoSocket == NULL)
        return;

    if(m_faceRequestSocket)
        stop();

    m_faceRequestSocket = new FaceRectReceiveSocket;
    m_faceRequestSocket->setInfo(m_serverInfoSocket, m_chanelIndex);
    m_faceRequestSocket->slotReconnect();

    connect(m_faceRequestSocket, SIGNAL(receivedFaceResult(QVector<QRect>, qint64, qint64)), this, SLOT(slotFaceResults(QVector<QRect>, qint64, qint64)));

    m_ipCamera->setReconnect(0);
    m_ipCamera->open(m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].streamuri);                
}

void CameraSurfaceGLView::stop()
{
    if(m_faceRequestSocket)
    {
        m_faceRequestSocket->stopSocket();
        delete m_faceRequestSocket;
        m_faceRequestSocket = NULL;
    }

    m_ipCamera->close();

    setEmpty();
}

ServerInfoSocket* CameraSurfaceGLView::serverInfoSocket()
{
    return m_serverInfoSocket;
}

int CameraSurfaceGLView::chanelIndex()
{
    return m_chanelIndex;
}

int CameraSurfaceGLView::isRunning()
{
    return m_ipCamera->isRunning();
}

void CameraSurfaceGLView::setMaximum(int maximum)
{
    m_isMaximumScreen = maximum;
    if(m_isMaximumScreen == 0)
    {
        m_maximumButton->setButtonPix(m_maximumNormalPix, m_maximumUpPix, m_maximumDownPix);
    }
    else
    {
        m_maximumButton->setButtonPix(m_tileNormalPix, m_tileUpPix, m_tileDownPix);
    }
    update();
}

int CameraSurfaceGLView::isMaximum()
{
    return m_isMaximumScreen;
}

void CameraSurfaceGLView::setSelected(int select)
{
    m_selected = select;
    update();
}

int CameraSurfaceGLView::isSelected()
{
    return m_selected;
}

QImage CameraSurfaceGLView::currentImage()
{
    if(m_stackFrameData.size() == 0)
        return QImage();

    QByteArray yuvData = m_stackFrameData[m_stackFrameData.size() - 1];

    QImage currentImage(m_frameWidth, m_frameHeight, QImage::Format_RGB888);
    convertYUV420P_toRGB888((unsigned char*)yuvData.data(), m_frameWidth, m_frameHeight, currentImage.bits());

    return currentImage;
}

void CameraSurfaceGLView::slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus)
{
    if(m_chanelIndex != chanelIndex)
        return;

    if(m_chanelStatus == chanelStatus)
        return;

    m_chanelStatus = chanelStatus;
    if(m_chanelStatus == CHANEL_STATUS_STOP)
        stop();
    else
        start();
}

void CameraSurfaceGLView::slotFaceResults(QVector<QRect> faceResults, qint64 serverTime, qint64 diffTime)
{
    qint64 curTime = serverTime - diffTime;
    if(m_stackTime.size() == 0)
    {
        setEmpty();
        return;
    }
    int index = 0;
    for(int i = 0; i < m_stackTime.size() - 1; i ++)
    {
        if(curTime >= m_stackTime[i] && curTime <= m_stackTime[i + 1])
        {
            index = i;
            if(m_stackTime[i + 1] - curTime < curTime - m_stackTime[i + 1])
                index = i + 1;

            break;
        }
    }

    if(curTime < m_stackTime[0])
    {
        index = 0;
    }
    else if(curTime > m_stackTime[m_stackTime.size() - 1])
    {
        index = m_stackTime.size() - 1;
    }

    m_frameImageData = m_stackFrameData[index];
    m_faceResults = faceResults;

#ifndef _IMAGE_HALF_
    QImage frameImage = QImage(m_frameWidth, m_frameHeight, QImage::Format_RGB888);
    convertYUV420P_toRGB888((unsigned char*)m_frameImageData.data(), m_frameWidth, m_frameHeight, frameImage.bits());
#else
    QImage frameImage = QImage(m_frameWidth / 2, m_frameHeight / 2, QImage::Format_RGB888);
    convertYUV420P_toRGB888_Scale4((unsigned char*)m_frameImageData.data(), m_frameWidth, m_frameHeight, frameImage.bits());
#endif
    m_frameImage = frameImage;

    update();
}

void CameraSurfaceGLView::slotFrameChanged()
{
#define MAX_STACK_SIZE 15
    QByteArray yuvData;
    qint64 frameTime;
    m_ipCamera->getCurFrame(yuvData, m_frameWidth, m_frameHeight, frameTime);
    if(yuvData.size() == 0 || !m_ipCamera->isRunning())
        return;

    m_stackFrameData.append(yuvData);
    m_stackTime.append(frameTime);
    if(m_stackFrameData.size() > MAX_STACK_SIZE)
    {
        m_stackFrameData.remove(0, m_stackFrameData.size() - MAX_STACK_SIZE);
        m_stackTime.remove(0, m_stackTime.size() - MAX_STACK_SIZE);
    }
}

void CameraSurfaceGLView::setEmpty()
{
    m_stackFrameData.clear();
    m_stackTime.resize(0);
    m_faceResults.resize(0);
    m_frameImageData.resize(0);
    m_frameImage = QImage();

    update();
}

void CameraSurfaceGLView::mousePressEvent(QMouseEvent* e)
{
    QWidget::mousePressEvent(e);

    emit selected(this);
    update();
}



void CameraSurfaceGLView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QWidget::mouseDoubleClickEvent(e);

    if(m_serverInfoSocket)
    {
        if(isMaximum())
            setMaximum(0);
        else
            setMaximum(1);

        emit maximumChanged();
    }
}


void CameraSurfaceGLView::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    QRect closeRect = QRect(rect().right() - 20, rect().top() + 7, m_closeNormalPix.width(), m_closeNormalPix.height());
    QRect maximumRect = QRect(rect().right() - 33, rect().top() + 7, m_maximumNormalPix.width(), m_maximumNormalPix.height());

    m_maximumButton->setGeometry(maximumRect);
    m_closeButton->setGeometry(closeRect);
}


void CameraSurfaceGLView::paintEvent(QPaintEvent *e)
{
    if(!isVisible())
        return;

    int yuvDrawFlag = 0;
    QPainter painter;
    painter.begin(this);

    if(!m_frameImage.isNull() && m_chanelStatus != CHANEL_STATUS_STOP)
    {
        painter.fillRect(rect(), Qt::black);
        QRect frameRect = getFrameRect(rect(), m_frameImage.size());
        painter.drawImage(frameRect, m_frameImage);

        yuvDrawFlag = 1;
    }

    if(yuvDrawFlag)
    {
        QVector<QRect> faceResults = m_faceResults;
        QRect frameRect = getFrameRect(rect(), QSize(m_frameWidth, m_frameHeight));

        for(int i = 0; i < faceResults.size(); i ++)
        {
            QRect faceRect = faceResults[i];
            QColor faceRectColor;
            if(faceRect.width() * faceRect.height() < 110 * 110)
                faceRectColor = Qt::red;
            else if(faceRect.width() * faceRect.height() < 160 * 160)
                faceRectColor = Qt::blue;
            else
                faceRectColor = Qt::green;

#ifndef _IMAGE_HALF_
            faceRect = QRect(frameRect.left() + faceRect.left() * frameRect.width() / (float)m_frameWidth,
                             frameRect.top() + faceRect.top() * frameRect.height() / (float)m_frameHeight,
                             faceRect.width() * frameRect.width() / (float)m_frameWidth,
                             faceRect.height() * frameRect.height() / (float)m_frameHeight);
#else
            faceRect = QRect(frameRect.left() + (faceRect.left() / 2) * frameRect.width() / (float)m_frameImage.width(),
                             frameRect.top() + (faceRect.top() / 2) * frameRect.height() / (float)m_frameImage.height(),
                             faceRect.width() * (frameRect.width() / 2) / (float)m_frameImage.width(),
                             faceRect.height() * (frameRect.height() / 2) / (float)m_frameImage.height());
#endif

            painter.save();
            painter.setPen(QPen(faceRectColor, 1));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawRect(faceRect);

            painter.setBrush(faceRectColor);

            qreal rounded = (qreal)faceRect.width() / 80;
            QRect borderRect(faceRect.left(), faceRect.top() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 4, faceRect.top() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left(), faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 4, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.top(), faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 4, faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.top(), faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 4, faceRect.width() / 50, faceRect.height() / 4);
            painter.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.top() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.top() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            painter.drawRect(borderRect);

            painter.restore();

        }

        painter.save();
        if(isSelected())
            painter.setPen(QPen(QColor(0, 255, 0), 2));
        else
            painter.setPen(QPen(QColor(100, 100, 100), 2));
        QRect boundingRect(rect().left() + 1, rect().top() + 1, rect().width() - 2, rect().height() - 2);
        painter.drawRect(boundingRect);

        painter.restore();

#ifdef SEND_SCORE
        painter.save();
       QFont font = painter.font();
       font.setPointSize(100);
       painter.setFont(font);
       painter.setPen(Qt::green);
       painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(g_faceScore));
       painter.restore();
#endif
    }
    else
    {
        painter.save();
        painter.drawPixmap(rect(), m_cameraNonePix);
        painter.restore();

        painter.save();
        if(isSelected())
            painter.setPen(QPen(QColor(0, 255, 0), 2));
        else
            painter.setPen(QPen(QColor(100, 100, 100), 2));
        QRect boundingRect(rect().left() + 1, rect().top() + 1, rect().width() - 2, rect().height() - 2);
        painter.drawRect(boundingRect);

        painter.restore();
    }

    if(m_serverInfoSocket)
    {
        ServerInfo serverInfo = m_serverInfoSocket->serverInfo();

        painter.save();

        QString cameraStr;
        cameraStr.sprintf("%s - %s (%s %d)", m_monitoringAreaName.toUtf8().data(), serverInfo.serverName.toUtf8().data(),
                          StringTable::Str_Chanel.toUtf8().data(), m_chanelIndex + 1);
        QRect cameraStrRect(rect().left() + 10, rect().top() + 10, rect().width() - 10, 30);

        QFont cameraFont = painter.font();
        cameraFont.setPointSize(10);

        painter.setFont(cameraFont);
        painter.setPen(Qt::red);
        painter.drawText(cameraStrRect, Qt::AlignLeft | Qt::AlignVCenter, cameraStr);

        painter.restore();

        if(m_chanelStatus == CHANEL_STATUS_STOP)
        {
            painter.save();

            QFont cameraFont = painter.font();
            cameraFont.setPointSize(14);

            painter.setFont(cameraFont);
            painter.setPen(Qt::red);
            painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, StringTable::Str_Disconnected);

            painter.restore();
        }
    }

    painter.end();
}


void  CameraSurfaceGLView::slotMaximumClicked()
{
    if(m_serverInfoSocket)
    {
        if(isMaximum())
            setMaximum(0);
        else
            setMaximum(1);

        emit maximumChanged();
    }
}

void CameraSurfaceGLView::slotCloseClicked()
{
    stopMonitoring();
    emit closeClicked(this);

    if(isMaximum())
    {
        setMaximum(0);
        emit maximumChanged();
    }
}

#endif
