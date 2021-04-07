#ifndef CAMERASURFACEGLVIEW_H
#define CAMERASURFACEGLVIEW_H

#include <QGLWidget>
#include <QtWidgets>

#include "ipcamera.h"
#include "base.h"


class ServerInfoSocket;
class FaceRectReceiveSocket;
class ControlButton;

#ifdef USE_GL

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class CameraSurfaceGLView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit CameraSurfaceGLView(QWidget *parent = 0);
    ~CameraSurfaceGLView();

    void                startMonitoring(ServerInfoSocket* serverInfoSocket, int chanelIndex, QString monitoringAreaName, int mediaIndex);

    void                stopMonitoring();

    void                start();
    void                stop();

    ServerInfoSocket*    serverInfoSocket();
    int                 chanelIndex();

    int                 isRunning();


    void                setMaximum(int maximum);
    int                 isMaximum();

    void                setSelected(int select);
    int                 isSelected();

    QImage              currentImage();
signals:
    void                closeClicked(CameraSurfaceGLView*);
    void                selected(CameraSurfaceGLView*);
    void                maximumChanged();
    void                frameChanged(QImage, qint64);

public slots:
    void                slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus);
    void                slotFaceResults(QVector<QRect> faceResults, qint64 engineTime, qint64 diffTime);
    void                slotFrameChanged(YUVFrame);

    void                slotMaximumClicked();
    void                slotCloseClicked();

protected:
    void                setEmpty();

    void                mousePressEvent(QMouseEvent*);
    void                mouseDoubleClickEvent(QMouseEvent*);

    void                initializeGL();
    void                resizeGL(int width, int height);
    void                paintEvent(QPaintEvent *e);

    void                createGL(int width, int height);
    void                releaseGL();
    void                resizeGL();

    bool                setupTextures();
    bool                setupShader();
    void                setYPixels(uint8_t* pixels, int stride);
    void                setUPixels(uint8_t* pixels, int stride);
    void                setVPixels(uint8_t* pixels, int stride);
private:
    ServerInfoSocket*   m_serverInfoSocket;
    int                 m_chanelIndex;
    int                 m_chanelStatus;
    FaceRectReceiveSocket*  m_faceRequestSocket;
    QString             m_monitoringAreaName;

    QVector<QByteArray> m_stackFrameData;
    QVector<qint64>     m_stackTime;
    QVector<QVector<QRect> > m_stackResults;
    QVector<qint64>         m_stackServerTimes;

    QByteArray          m_frameImageData;
    int                 m_frameWidth;
    int                 m_frameHeight;

    QVector<QRect>      m_faceResults;

    QPixmap             m_closeNormalPix;
    QPixmap             m_closeUpPix;
    QPixmap             m_closeDownPix;

    QPixmap             m_maximumNormalPix;
    QPixmap             m_maximumUpPix;
    QPixmap             m_maximumDownPix;

    QPixmap             m_tileNormalPix;
    QPixmap             m_tileUpPix;
    QPixmap             m_tileDownPix;

    int                 m_isMaximumScreen;

    IpCamera*           m_ipCamera;
    int                 m_mediaIndex;

    int                 m_running;
    QPixmap             m_cameraNonePix;

    int                 m_selected;
    int                 m_create;

    int                 m_initialized;

    ControlButton*      m_maximumButton;
    ControlButton*      m_closeButton;

    int vid_w;
    int vid_h;
    int win_w;
    int win_h;
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
};
#else
class CameraSurfaceGLView : public QWidget
{
    Q_OBJECT
public:
    explicit CameraSurfaceGLView(QWidget *parent = 0);
    ~CameraSurfaceGLView();

    void                startMonitoring(ServerInfoSocket* serverInfoSocket, int chanelIndex, QString monitoringAreaName, int mediaIndex);
    void                stopMonitoring();

    void                start();
    void                stop();

    ServerInfoSocket*    serverInfoSocket();
    int                 chanelIndex();

    int                 isRunning();


    void                setMaximum(int maximum);
    int                 isMaximum();

    void                setSelected(int select);
    int                 isSelected();

    QImage              currentImage();

signals:
    void                closeClicked(CameraSurfaceGLView*);
    void                selected(CameraSurfaceGLView*);
    void                maximumChanged();
    void                frameChanged(QImage, qint64);

public slots:
    void                slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus);
    void                slotFaceResults(QVector<QRect> faceResults, qint64 engineTime, qint64 diffTime);
    void                slotFrameChanged();

    void                slotMaximumClicked();
    void                slotCloseClicked();

protected:
    void                setEmpty();

    void                mousePressEvent(QMouseEvent*);
    void                mouseDoubleClickEvent(QMouseEvent*);

    void                paintEvent(QPaintEvent *e);
    void                resizeEvent(QResizeEvent *);
private:
    ServerInfoSocket*   m_serverInfoSocket;
    int                 m_chanelIndex;
    int                 m_chanelStatus;
    FaceRectReceiveSocket*  m_faceRequestSocket;
    QString             m_monitoringAreaName;

    QVector<QByteArray> m_stackFrameData;
    QVector<qint64>     m_stackTime;
    QVector<QVector<QRect> > m_stackResults;
    QVector<qint64>         m_stackServerTimes;

    QByteArray          m_frameImageData;
    int                 m_frameWidth;
    int                 m_frameHeight;
    QImage              m_frameImage;

    QVector<QRect>      m_faceResults;

    QPixmap             m_closeNormalPix;
    QPixmap             m_closeUpPix;
    QPixmap             m_closeDownPix;

    QPixmap             m_maximumNormalPix;
    QPixmap             m_maximumUpPix;
    QPixmap             m_maximumDownPix;

    QPixmap             m_tileNormalPix;
    QPixmap             m_tileUpPix;
    QPixmap             m_tileDownPix;

    int                 m_isMaximumScreen;

    IpCamera*           m_ipCamera;
    int                 m_mediaIndex;

    int                 m_running;
    QPixmap             m_cameraNonePix;

    int                 m_selected;

    ControlButton*      m_maximumButton;
    ControlButton*      m_closeButton;
};

#endif

#endif // CAMERASURFACEGLVIEW_H
