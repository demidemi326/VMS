#include "mediaitem.h"
#include "clientbase.h"
#include "facerequestsocket.h"
#include "serverinfosocket.h"
#include "stringtable.h"
#include "ipcamera.h"
#include "camerasurfaceglitem.h"

#include <QtWidgets>

extern int g_fontId;

void convertYUV420P_toRGB888(unsigned char* data, int width, int height, unsigned char* dstData);
void convertYUV420P_toRGB888_Scale4(unsigned char* data, int width, int height, unsigned char* dstData);

MediaItem::MediaItem(QObject *parent)
{
    m_frameWidth = 1;
    m_frameHeight = 1;
    m_chanelIndex = 0;
    m_mediaIndex = 0;
    m_surfaceItem = NULL;

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

    m_closeState = BTN_STATUS_NORMAL;
    m_maximumState = BTN_STATUS_NORMAL;
    m_isMaximumScreen = 0;
    m_mouseState = 0;
    m_hover = 0;

    m_chanelStatus = CHANEL_STATUS_STOP;

    m_ipCamera = new IpCamera;
    qRegisterMetaType<YUVFrame>("YUVFrame");

    connect(m_ipCamera, SIGNAL(frameChanged(YUVFrame)), this, SLOT(slotFrameChanged(YUVFrame)));

    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

MediaItem::~MediaItem()
{

}

void MediaItem::startMonitoring(ServerInfoSocket* serverInfoSocket, int chanelIndex, QString monitoringAreaName, int mediaIndex,
                                CameraSurfaceGLItem* surfaceItem)
{
    stopMonitoring();

    m_monitoringAreaName = monitoringAreaName;
    m_serverInfoSocket = serverInfoSocket;
    m_chanelIndex = chanelIndex;
    m_mediaIndex = mediaIndex;
    m_surfaceItem = surfaceItem;

    m_chanelStatus = m_serverInfoSocket->getChanelStatus(chanelIndex);
    connect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int, int, int)));

    if(m_chanelStatus != CHANEL_STATUS_STOP)
        start();
}

void MediaItem::stopMonitoring()
{
    stop();

    if(m_serverInfoSocket)
    {
        disconnect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int, int, int)));
        m_serverInfoSocket = NULL;
    }

    m_surfaceItem = NULL;

    m_monitoringAreaName = "";
}

void MediaItem::start()
{
    if(m_serverInfoSocket == NULL)
        return;

    if(m_faceRequestSocket)
        stop();

    m_faceRequestSocket = new FaceRectReceiveSocket;
    m_faceRequestSocket->setInfo(m_serverInfoSocket, m_chanelIndex);
    m_faceRequestSocket->slotReconnect();

    connect(m_faceRequestSocket, SIGNAL(receivedFaceResult(QVector<QRect>, qint64, qint64)), this, SLOT(slotFaceResults(QVector<QRect>, qint64, qint64)));

//    m_ipCamera->open(m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].ipAddress, m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].userName,
//                     m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].password);
    m_ipCamera->open(m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex].streamuri);
}

void MediaItem::stop()
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


void MediaItem::slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus)
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

ServerInfoSocket* MediaItem::serverInfoSocket()
{
    return m_serverInfoSocket;
}

int MediaItem::chanelIndex()
{
    return m_chanelIndex;
}

int MediaItem::isRunning()
{
    return m_ipCamera->isRunning();
}


void MediaItem::setEmpty()
{
    m_stackFrameData.clear();
    m_stackTime.resize(0);
    m_faceResults.resize(0);
    m_frameImageData.resize(0);

    update();
}

void MediaItem::setBoundingRect(QRect rect)
{
    m_boundingRect = rect;
    m_closeRect = QRect(m_boundingRect.right() - 20, m_boundingRect.top() + 7, m_closeNormalPix.width(), m_closeNormalPix.height());
    m_maximumRect = QRect(m_boundingRect.right() - 33, m_boundingRect.top() + 7, m_maximumNormalPix.width(), m_maximumNormalPix.height());
}

QRectF MediaItem::boundingRect() const
{
    return m_boundingRect;
}

void MediaItem::setMaximum(int maximum)
{
    m_isMaximumScreen = maximum;
    update();
}

int MediaItem::isMaximum()
{
    return m_isMaximumScreen;
}

void MediaItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    return;
    qDebug() << "fdsafsadf" << m_frameImageData.size();

    QVector<QRect> faceResults = m_faceResults;
    if(m_frameImageData.size() && m_chanelStatus != CHANEL_STATUS_STOP && m_ipCamera->isRunning())
    {
        QPixmap memPix(m_boundingRect.size());
        QPainter memDC;
        memDC.begin(&memPix);

        QRect frameRect = getFrameRect(memPix.rect(), QSize(m_frameWidth, m_frameHeight));

//        memDC.fillRect(m_boundingRect, Qt::black);

        for(int i = 0; i < faceResults.size(); i ++)
        {
            memDC.save();

            QRect faceRect = faceResults[i];

#ifndef _IMAGE_HALF_
            faceRect = QRect(frameRect.left() + faceRect.left() * frameRect.width() / (float)m_frameWidth,
                             frameRect.top() + faceRect.top() * frameRect.height() / (float)m_frameHeight,
                             faceRect.width() * frameRect.width() / (float)m_frameWidth,
                             faceRect.height() * frameRect.height() / (float)m_frameHeight);
#else
            faceRect = QRect(frameRect.left() + (faceRect.left() / 2) * frameRect.width() / (float)m_frameWidth,
                             frameRect.top() + (faceRect.top() / 2) * frameRect.height() / (float)m_frameHeight,
                             faceRect.width() * (frameRect.width() / 2) / (float)m_frameWidth,
                             faceRect.height() * (frameRect.height() / 2) / (float)m_frameHeight);
#endif


            memDC.setPen(QPen(QColor(Qt::green), 1));
            memDC.setRenderHint(QPainter::Antialiasing);
            memDC.drawRect(faceRect);

            memDC.setBrush(QColor(Qt::green));

            qreal rounded = (qreal)faceRect.width() / 80;
            QRect borderRect(faceRect.left(), faceRect.top() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 4, faceRect.top() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left(), faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 4, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 4, faceRect.height() / 50);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.top(), faceRect.width() / 50, faceRect.height() / 4);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 4, faceRect.width() / 50, faceRect.height() / 4);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.top(), faceRect.width() / 50, faceRect.height() / 4);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 4, faceRect.width() / 50, faceRect.height() / 4);
            memDC.drawRoundedRect(borderRect, rounded, rounded);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.top() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            memDC.drawRect(borderRect);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.top() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            memDC.drawRect(borderRect);

            borderRect = QRect(faceRect.left() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            memDC.drawRect(borderRect);

            borderRect = QRect(faceRect.right() - faceRect.width() / 100, faceRect.bottom() - faceRect.height() / 100, faceRect.width() / 50, faceRect.height() / 50);
            memDC.drawRect(borderRect);

            memDC.restore();

        }

        memDC.end();

        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(0, 0,memPix);

        painter->save();
        if(isSelected())
        {
            painter->setPen(QPen(QColor(0, 255, 0), 2));
        }
        else
        {
            painter->setPen(QPen(QColor(100, 100, 100), 2));
        }
        QRect boundingRect(m_boundingRect.left() + 1, m_boundingRect.top() + 1, m_boundingRect.width() - 2, m_boundingRect.height() - 2);
        painter->drawRect(boundingRect);

        painter->restore();
    }
    else
    {
        //painter->fillRect(m_boundingRect, QColor(200, 200, 200));
        painter->save();
        painter->setOpacity(0.8);
        painter->drawPixmap(m_boundingRect, QPixmap(":/images/camera_none.png"));

        painter->restore();

        painter->save();
        if(isSelected())
            painter->setPen(QPen(QColor(0, 255, 0), 2));
        else
            painter->setPen(QPen(QColor(100, 100, 100), 2));
        QRect boundingRect(m_boundingRect.left() + 1, m_boundingRect.top() + 1, m_boundingRect.width() - 2, m_boundingRect.height() - 2);
        painter->drawRect(boundingRect);

        painter->restore();
    }

    if(m_serverInfoSocket)
    {
        ServerInfo serverInfo = m_serverInfoSocket->serverInfo();

        painter->save();

        QString cameraStr;
        cameraStr.sprintf("%s - %s (%s %d)", m_monitoringAreaName.toUtf8().data(), serverInfo.serverName.toUtf8().data(),
                          StringTable::Str_Chanel.toUtf8().data(), m_chanelIndex + 1);
        QRect cameraStrRect(m_boundingRect.left() + 10, m_boundingRect.top() + 10, m_boundingRect.width() - 10, 30);

        QFont cameraFont = painter->font();
        cameraFont.setPointSize(10);

        painter->setFont(cameraFont);
        painter->setPen(Qt::red);
        painter->drawText(cameraStrRect, Qt::AlignLeft | Qt::AlignVCenter, cameraStr);

        painter->restore();

        if(m_chanelStatus == CHANEL_STATUS_STOP)
        {
            painter->save();

            QFont cameraFont = painter->font();
            cameraFont.setPointSize(14);

            painter->setFont(cameraFont);
            painter->setPen(Qt::red);
            painter->drawText(m_boundingRect, Qt::AlignHCenter | Qt::AlignVCenter, StringTable::Str_Disconnected);

            painter->restore();
        }

        if(m_closeState == BTN_STATUS_NORMAL)
            painter->drawPixmap(m_closeRect, m_closeNormalPix);
        else if(m_closeState == BTN_STATUS_DOWN)
            painter->drawPixmap(m_closeRect, m_closeDownPix);
        else
            painter->drawPixmap(m_closeRect, m_closeUpPix);

        if(m_maximumState == BTN_STATUS_NORMAL)
        {
            if(m_isMaximumScreen == 0)
                painter->drawPixmap(m_maximumRect, m_maximumNormalPix);
            else
                painter->drawPixmap(m_maximumRect, m_tileNormalPix);
        }
        else if(m_maximumState == BTN_STATUS_DOWN)
        {
            if(m_isMaximumScreen == 0)
                painter->drawPixmap(m_maximumRect, m_maximumDownPix);
            else
                painter->drawPixmap(m_maximumRect, m_tileDownPix);
        }
        else
        {
            if(m_isMaximumScreen == 0)
                painter->drawPixmap(m_maximumRect, m_maximumUpPix);
            else
                painter->drawPixmap(m_maximumRect, m_tileUpPix);
        }
    }
}


void MediaItem::slotFrameChanged(YUVFrame yuvFrame)
{
    if(yuvFrame.yuvData == 0 || !m_ipCamera->isRunning())
    {
        m_frameImageData.clear();
        update();
        return;
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

void MediaItem::slotFaceResults(QVector<QRect> faceResults, qint64 serverTime, qint64 diffTime)
{
    qint64 curTime = serverTime - diffTime;

    qDebug() << "aaa" << m_stackFrameData.size() << m_stackTime.size();
    if(m_stackFrameData.size() == 0)
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

//    m_frameImage = m_stackFrames[index];
    m_frameImageData = m_stackFrameData[index];
    m_faceResults = faceResults;

    update();
}


void MediaItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem::mousePressEvent(e);
    m_mouseState = 1;
    if(m_closeRect.contains(e->pos().toPoint()))
        m_closeState = BTN_STATUS_DOWN;

    if(m_maximumRect.contains(e->pos().toPoint()))
        m_maximumState = BTN_STATUS_DOWN;

    update();
}

void MediaItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem::mouseReleaseEvent(e);
    if(m_closeRect.contains(e->pos().toPoint()))
    {
        m_closeState = BTN_STATUS_UP;
        stopMonitoring();
        emit closeClicked(this);

        if(isMaximum())
        {
            setMaximum(0);
            emit maximumChanged();
        }
    }
    else
        m_closeState = BTN_STATUS_NORMAL;

    if(m_maximumRect.contains(e->pos().toPoint()) && m_serverInfoSocket)
    {
        m_maximumState = BTN_STATUS_UP;
        if(isMaximum())
            setMaximum(0);
        else
            setMaximum(1);

        emit maximumChanged();
    }
    else
        m_maximumState = BTN_STATUS_NORMAL;

    m_mouseState = 0;

    update();
}

void MediaItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem::mouseDoubleClickEvent(e);

    if(m_serverInfoSocket)
    {
        if(isMaximum())
            setMaximum(0);
        else
            setMaximum(1);

        emit maximumChanged();
    }
}

void MediaItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverEnterEvent(event);
}

void MediaItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverLeaveEvent(event);

    m_closeState = BTN_STATUS_NORMAL;
    m_maximumState = BTN_STATUS_NORMAL;
    update();
}

void MediaItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverMoveEvent(event);

    if(m_closeRect.contains(event->pos().toPoint()) && m_mouseState == 1)
        m_closeState = BTN_STATUS_DOWN;
    else if(m_closeRect.contains(event->pos().toPoint()) && m_mouseState == 0)
        m_closeState = BTN_STATUS_UP;
    else
        m_closeState = BTN_STATUS_NORMAL;

    if(m_maximumRect.contains(event->pos().toPoint()) && m_mouseState == 1)
        m_maximumState = BTN_STATUS_DOWN;
    else if(m_maximumRect.contains(event->pos().toPoint()) && m_mouseState == 0)
        m_maximumState = BTN_STATUS_UP;
    else
        m_maximumState = BTN_STATUS_NORMAL;
    update();
}


QImage MediaItem::currentImage()
{
    if(m_stackFrameData.size() == 0)
        return QImage();

    QByteArray yuvData = m_stackFrameData[m_stackFrameData.size() - 1];

    QImage currentImage(m_frameWidth, m_frameHeight, QImage::Format_RGB888);
    convertYUV420P_toRGB888((unsigned char*)yuvData.data(), m_frameWidth, m_frameHeight, currentImage.bits());

    return currentImage;
}


int convertYUVtoARGB(int Y, int U, int V, unsigned char* dstData, int index)
{
    int C = qMax(0, Y - 16);
    int D = U;
    int E = V;

    int r = (298 * C  + 409 * E + 128) >> 8;
    int g = (298 * C - 100 * D - 208 * E + 128) >> 8;
    int b = (298 * C + 516 * D + 128) >> 8;

    r = r>255? 255 : r<0 ? 0 : r;
    g = g>255? 255 : g<0 ? 0 : g;
    b = b>255? 255 : b<0 ? 0 : b;

    dstData[index * 3] = r;
    dstData[index * 3 + 1] = g;
    dstData[index * 3 + 2] = b;

    return 0;
}

void convertYUV420P_toRGB888(unsigned char* data, int width, int height, unsigned char* dstData)
{
    int size = width*height;
    int offset = size;
    int u, v, y1, y2, y3, y4;

    for(int i = 0, k = offset; i < size; i += 2, k ++)
    {
        y1 = data[i];
        y2 = data[i + 1];
        y3 = data[width + i];
        y4 = data[width + i + 1];

        v = data[k + width * height / 4];
        u = data[k];
        v = v-128;
        u = u-128;

        convertYUVtoARGB(y1, u, v, dstData, i);
        convertYUVtoARGB(y2, u, v, dstData, i + 1);
        convertYUVtoARGB(y3, u, v, dstData, width + i);
        convertYUVtoARGB(y4, u, v, dstData, width + i + 1);

        if (i!=0 && (i+2)%width==0)
            i += width;
    }
}


void convertYUV420P_toRGB888_Scale4(unsigned char* data, int width, int height, unsigned char* dstData)
{
    int size = width * height;
    int offset = size;
    int u, v, y1, y2, y3, y4;

    int newWidth = width / 2;
    int newHeight = height / 2;

    for(int y = 0; y < newHeight; y ++)
    {
        for(int x = 0; x < newWidth; x ++)
        {
            y1 = data[y * 2 * width + x * 2];
            u = data[offset + (y * (width / 2)) + x];
            v = data[offset + width * height / 4 + (y * (width / 2)) + x];
            v = v-128;
            u = u-128;

            convertYUVtoARGB(y1, u, v, dstData, y * newWidth + x);
        }
    }
}


void receivedFrameProc(int sender, YUVFrame yuvFrame)
{
    MediaItem* mediaItem = (MediaItem*)sender;
    mediaItem->slotFrameChanged(yuvFrame);
}
