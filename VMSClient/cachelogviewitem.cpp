#include "cachelogviewitem.h"
#include "clientbase.h"

#include <QtWidgets>

CacheLogViewItem::CacheLogViewItem(QObject *parent)
{
    m_boundingRect = QRect(0, 0, CACHE_LOG_ITEM_WIDTH, CACHE_LOG_ITEM_HEIGHT);

    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF CacheLogViewItem::boundingRect() const
{
    if(m_boundingRect.isNull())
        return QRectF(QPointF(0,0), QPointF(0,0));

    return m_boundingRect;
}

void CacheLogViewItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->fillRect(m_boundingRect, Qt::white);

    if(m_logResult.faceImage.size())
    {
        QImage faceImage = QImage::fromData(m_logResult.faceImage);
        QRect faceRect = getFrameRect(QRect(0, 0, CACHE_LOG_ITEM_WIDTH, CACHE_LOG_ITEM_WIDTH), faceImage.size());
        QPixmap memPix(CACHE_LOG_ITEM_WIDTH, CACHE_LOG_ITEM_HEIGHT);
        QPainter memDC;
        memDC.begin(&memPix);

        memDC.fillRect(memPix.rect(), Qt::white);
        memDC.drawImage(faceRect, faceImage);

        if(isSelected())
        {
            memDC.save();
            memDC.setOpacity(0.3);
            memDC.fillRect(memPix.rect(), Qt::blue);
            memDC.restore();
        }

        QRect dateRect(0, CACHE_LOG_ITEM_WIDTH, CACHE_LOG_ITEM_WIDTH, CACHE_LOG_TEXT_HEIGHT);
        QRect timeRect(0, CACHE_LOG_ITEM_WIDTH + CACHE_LOG_TEXT_HEIGHT, CACHE_LOG_ITEM_WIDTH, CACHE_LOG_TEXT_HEIGHT);
        QRect serverRect(0, CACHE_LOG_ITEM_WIDTH + CACHE_LOG_TEXT_HEIGHT * 2, CACHE_LOG_ITEM_WIDTH, CACHE_LOG_TEXT_HEIGHT);

        memDC.drawText(dateRect, Qt::AlignHCenter | Qt::AlignVCenter, m_logResult.dateTime.date().toString("yyyy-MM-dd"));
        memDC.drawText(timeRect, Qt::AlignHCenter | Qt::AlignVCenter, m_logResult.dateTime.time().toString("hh:mm:ss"));
        memDC.drawText(serverRect, Qt::AlignHCenter | Qt::AlignVCenter, m_logResult.areaName);

        memDC.end();

        painter->drawPixmap(m_boundingRect, memPix);
    }
}

void CacheLogViewItem::setBoundingRect(QRect rect)
{
    m_boundingRect = rect;
}

void CacheLogViewItem::setInfo(LOG_RESULT logResult)
{
    m_logResult = logResult;
}

void CacheLogViewItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem::mouseDoubleClickEvent(e);

    emit logItemDoubleClicked(m_logResult);
}
