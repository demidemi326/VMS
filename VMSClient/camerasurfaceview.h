#ifndef CAMERASURFACEVIEW_H
#define CAMERASURFACEVIEW_H

#include <QWidget>
#include <QtWidgets>

#include "ipcamera.h"

class ServerInfoSocket;
class FaceRectReceiveSocket;
class ControlButton;
class CameraSurfaceView : public QWidget
{
    Q_OBJECT
public:
    explicit CameraSurfaceView(QWidget *parent = 0);
    ~CameraSurfaceView();

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
    void                closeClicked(CameraSurfaceView*);
    void                selected(CameraSurfaceView*);
    void                maximumChanged();
    void                frameChanged(QImage, qint64);

public slots:
    void                slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus);
    void                slotFaceResults(QVector<QRect> faceResults, qint64 engineTime, qint64 diffTime, QImage frameimage);

    void                slotMaximumClicked();
    void                slotCloseClicked();

protected:
    void                setEmpty();

    void                mousePressEvent(QMouseEvent*);
    void                mouseDoubleClickEvent(QMouseEvent*);

    void                paintEvent(QPaintEvent *);
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
    int                 m_create;

    ControlButton*      m_maximumButton;
    ControlButton*      m_closeButton;
};

#endif // CAMERASURFACEVIEW_H
