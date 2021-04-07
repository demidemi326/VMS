#include "camerasurfaceview.h"
#include "base.h"
#include "facerequestsocket.h"
#include "serverinfosocket.h"
#include "stringtable.h"
#include "ipcamera.h"
#include "controlbutton.h"

#include <QtWidgets>

CameraSurfaceView::CameraSurfaceView(QWidget *parent) : QWidget(parent)
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
    m_create = 0;

    m_chanelStatus = CHANEL_STATUS_STOP;

    m_maximumButton = new ControlButton(this);
    m_closeButton = new ControlButton(this);

    m_maximumButton->setButtonPix(m_maximumNormalPix, m_maximumUpPix, m_maximumDownPix);
    m_closeButton->setButtonPix(m_closeNormalPix, m_closeUpPix, m_closeDownPix);

    m_ipCamera = new IpCamera;
    qRegisterMetaType<YUVFrame>("YUVFrame");

//    connect(m_ipCamera, SIGNAL(frameChanged(YUVFrame)), this, SLOT(slotFrameChanged(YUVFrame)));
    connect(m_maximumButton, SIGNAL(clicked()), this, SLOT(slotMaximumClicked()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));
}

CameraSurfaceView::~CameraSurfaceView()
{

}


void CameraSurfaceView::startMonitoring(ServerInfoSocket* serverInfoSocket, int chanelIndex, QString monitoringAreaName, int mediaIndex)
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

void CameraSurfaceView::stopMonitoring()
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

void CameraSurfaceView::start()
{
    if(m_serverInfoSocket == NULL)
        return;

    if(m_faceRequestSocket)
        stop();

    m_faceRequestSocket = new FaceRectReceiveSocket;
    m_faceRequestSocket->setInfo(m_serverInfoSocket, m_chanelIndex);
    m_faceRequestSocket->slotReconnect();

    connect(m_faceRequestSocket, SIGNAL(receivedFaceResult(QVector<QRect>, qint64, qint64, QImage)), this, SLOT(slotFaceResults(QVector<QRect>, qint64, qint64, QImage)));
}

void CameraSurfaceView::stop()
{
    if(m_faceRequestSocket)
    {
        m_faceRequestSocket->stopSocket();
        delete m_faceRequestSocket;
        m_faceRequestSocket = NULL;
    }

    setEmpty();

}

ServerInfoSocket* CameraSurfaceView::serverInfoSocket()
{
    return m_serverInfoSocket;
}

int CameraSurfaceView::chanelIndex()
{
    return m_chanelIndex;
}

int CameraSurfaceView::isRunning()
{
    return m_ipCamera->isRunning();
}

void CameraSurfaceView::setMaximum(int maximum)
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

int CameraSurfaceView::isMaximum()
{
    return m_isMaximumScreen;
}

void CameraSurfaceView::setSelected(int select)
{
    m_selected = select;
    update();
}

int CameraSurfaceView::isSelected()
{
    return m_selected;
}

QImage CameraSurfaceView::currentImage()
{
    return m_frameImage;
}

void CameraSurfaceView::slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus)
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

void CameraSurfaceView::slotFaceResults(QVector<QRect> faceResults, qint64 engineTime, qint64 diffTime, QImage frameImage)
{
    m_frameImage = frameImage;
    m_faceResults = faceResults;

    update();
}


void CameraSurfaceView::slotMaximumClicked()
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

void CameraSurfaceView::slotCloseClicked()
{
    stopMonitoring();
    emit closeClicked(this);

    if(isMaximum())
    {
        setMaximum(0);
        emit maximumChanged();
    }
}

void CameraSurfaceView::setEmpty()
{
    m_stackFrameData.clear();
    m_stackTime.resize(0);
    m_faceResults.resize(0);
    m_frameImageData.resize(0);
    m_frameImage = QImage();

    update();
}

void CameraSurfaceView::mousePressEvent(QMouseEvent* e)
{
    QWidget::mousePressEvent(e);

    emit selected(this);
    update();
}

void CameraSurfaceView::mouseDoubleClickEvent(QMouseEvent* e)
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


void CameraSurfaceView::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if(!isVisible())
        return;

    QPainter painter;
    painter.begin(this);

    if(!m_frameImage.isNull() && m_chanelStatus != CHANEL_STATUS_STOP)
    {
        QPixmap memPix(rect().size());
        QPainter memDC;
        memDC.begin(&memPix);

        memDC.fillRect(rect(), Qt::black);

        QVector<QRect> faceResults = m_faceResults;
        QRect frameRect = getFrameRect(rect(), QSize(m_frameImage.width(), m_frameImage.height()));
        memDC.drawImage(frameRect, m_frameImage);

        for(int i = 0; i < faceResults.size(); i ++)
        {
            QRect faceRect = faceResults[i];

#ifndef _IMAGE_HALF_
            faceRect = QRect(frameRect.left() + faceRect.left() * frameRect.width() / (float)m_frameImage.width(),
                             frameRect.top() + faceRect.top() * frameRect.height() / (float)m_frameImage.height(),
                             faceRect.width() * frameRect.width() / (float)m_frameImage.width(),
                             faceRect.height() * frameRect.height() / (float)m_frameImage.height());
#else
            faceRect = QRect(frameRect.left() + (faceRect.left() / 2) * frameRect.width() / (float)m_frameImage.width(),
                             frameRect.top() + (faceRect.top() / 2) * frameRect.height() / (float)m_frameImage.height(),
                             faceRect.width() * (frameRect.width() / 2) / (float)m_frameImage.width(),
                             faceRect.height() * (frameRect.height() / 2) / (float)m_frameImage.height());
#endif

            memDC.save();
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

        painter.drawPixmap(rect(), memPix);

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

void CameraSurfaceView::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    QRect closeRect = QRect(rect().right() - 20, rect().top() + 7, m_closeNormalPix.width(), m_closeNormalPix.height());
    QRect maximumRect = QRect(rect().right() - 33, rect().top() + 7, m_maximumNormalPix.width(), m_maximumNormalPix.height());

    m_maximumButton->setGeometry(maximumRect);
    m_closeButton->setGeometry(closeRect);
}
