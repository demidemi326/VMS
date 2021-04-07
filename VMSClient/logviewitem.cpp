#include "logviewitem.h"
#include "clientbase.h"

#include <QtWidgets>

LogViewItem::LogViewItem(QObject *parent)
{
    m_boundingRect = QRect(0, 0, LOG_ITEM_WIDTH, LOG_ITEM_HEIGHT);

    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF LogViewItem::boundingRect() const
{
    if(m_boundingRect.isNull())
        return QRectF(QPointF(0,0), QPointF(0,0));

    return m_boundingRect;
}

void LogViewItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->fillRect(m_boundingRect, Qt::white);

    QImage faceImage = QImage::fromData(m_faceImageInfo.faceImage);
    QRect faceRect = getFrameRect(QRect(0, 25, LOG_IMAGE_WIDTH, LOG_IMAGE_HEIGHT), faceImage.size());
    QPixmap memPix(LOG_ITEM_WIDTH, LOG_ITEM_HEIGHT);
    QPainter memDC;
    memDC.begin(&memPix);

    memDC.fillRect(memPix.rect(), Qt::white);
    memDC.drawImage(faceRect, faceImage);

    for(int j = 0; j < 3; j ++)
    {
        QImage candImage = QImage::fromData(m_faceImageInfo.candidateFace[j], "JPG");
        if(candImage.isNull())
        {
            QRect candRect(LOG_IMAGE_WIDTH + 20 + j * (5 + LOG_CAND_IMAGE_WIDTH), 25,
                        LOG_CAND_IMAGE_WIDTH, LOG_CAND_IMAGE_HEIGHT);
            memDC.drawRect(candRect);
        }
        else
        {
            QRect candRect = getFrameRect(QRect(LOG_IMAGE_WIDTH + 20 + j * (5 + LOG_CAND_IMAGE_WIDTH), 25,
                                                LOG_CAND_IMAGE_WIDTH, LOG_CAND_IMAGE_HEIGHT), candImage.size());

            memDC.drawImage(candRect, candImage);

            memDC.save();
            if(m_faceImageInfo.candidateType[j] == 0)
                memDC.setPen(QPen(QColor(0, 255, 0), 3));
            else
                memDC.setPen(QPen(QColor(255, 0, 0), 3));
            memDC.drawRect(QRect(candRect.left(), candRect.top(), candRect.width() - 2, candRect.height() - 2));
            memDC.restore();

            QRect candScoreRect(LOG_IMAGE_WIDTH + 20 + j * (5 + LOG_CAND_IMAGE_WIDTH), 0,
                                LOG_CAND_IMAGE_WIDTH, 25);
            QString scoreStr;
            scoreStr.sprintf("%.3f", 1 - m_faceImageInfo.candidateDist[j]);

            memDC.save();
            memDC.setPen(Qt::red);
            memDC.drawText(candScoreRect, Qt::AlignHCenter | Qt::AlignVCenter, scoreStr);
            memDC.restore();
        }
    }

    if(isSelected())
    {
        memDC.save();
        memDC.setOpacity(0.3);
        memDC.fillRect(memPix.rect(), Qt::blue);
        memDC.restore();
    }

    QRect dateRect(0, LOG_ITEM_IMAGE_HEIGHT, LOG_ITEM_WIDTH, LOG_TEXT_HEIGHT);

    memDC.drawText(dateRect, Qt::AlignHCenter | Qt::AlignVCenter, m_faceImageInfo.areaName + " - " + m_faceImageInfo.dateTime.toString("yyyy-MM-dd hh:mm:ss" ));

    memDC.end();

    painter->drawPixmap(m_boundingRect, memPix);
}

void LogViewItem::setBoundingRect(QRect rect)
{
    m_boundingRect = rect;
}

void LogViewItem::setInfo(LOG_RESULT faceImageInfo)
{
    m_faceImageInfo = faceImageInfo;
}

void LogViewItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem::mouseDoubleClickEvent(e);

    emit logItemDoubleClicked(m_faceImageInfo);
}
