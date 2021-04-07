#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include <QGraphicsItem>
#include <QImage>
#include <QObject>
#include <QPixmap>
#include <QMutex>

#include "base.h"
#include "clientbase.h"
#include "ipcamera.h"

#define BTN_STATUS_NORMAL 0
#define BTN_STATUS_UP 1
#define BTN_STATUS_DOWN 2

class FaceRectReceiveSocket;
class ServerInfoSocket;
class CameraSurfaceGLItem;
class MediaItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit MediaItem(QObject *parent = 0);
    ~MediaItem();

    void                startMonitoring(ServerInfoSocket* serverInfoSocket, int chanelIndex, QString monitoringAreaName, int mediaIndex,
                                        CameraSurfaceGLItem* surfaceView);
    void                stopMonitoring();

    void                start();
    void                stop();

    ServerInfoSocket*    serverInfoSocket();
    int                 chanelIndex();

    int                 isRunning();

    void                setEmpty();

    void                setBoundingRect(QRect rect);
    QRectF              boundingRect() const;

    void                paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void                setMaximum(int maximum);
    int                 isMaximum();

    QImage              currentImage();

signals:
    void                closeClicked(MediaItem* );
    void                maximumChanged();
    void                frameChanged(QImage, qint64);

public slots:
    void                slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus);
    void                slotFaceResults(QVector<QRect> faceResults, qint64 engineTime, qint64 diffTime);
    void                slotFrameChanged(YUVFrame);

protected:
    void                mousePressEvent(QGraphicsSceneMouseEvent*);
    void                mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    void                mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);

    void                hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void                hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void                hoverMoveEvent(QGraphicsSceneHoverEvent *event);

private:
    QRect               m_boundingRect;

    CameraSurfaceGLItem*    m_surfaceItem;
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
    QRect               m_closeRect;

    QPixmap             m_maximumNormalPix;
    QPixmap             m_maximumUpPix;
    QPixmap             m_maximumDownPix;
    QRect               m_maximumRect;

    QPixmap             m_tileNormalPix;
    QPixmap             m_tileUpPix;
    QPixmap             m_tileDownPix;

    int                 m_closeState;
    int                 m_maximumState;
    int                 m_isMaximumScreen;
    int                 m_mouseState;
    int                 m_hover;

    QTime               m_oldTime;
    QImage              m_tmpImage;
    QMutex              m_mutex;

    IpCamera*       m_ipCamera;
    int             m_mediaIndex;

    int             m_running;

};
#endif // MEDIAITEM_H
