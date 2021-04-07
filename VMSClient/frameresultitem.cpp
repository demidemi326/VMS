#include "frameresultitem.h"
#include "clientbase.h"

#include <QtWidgets>

FrameResultItem::FrameResultItem(QObject *)
{
    m_boundingRect = QRect(0, 0, FRAME_RESULT_ITEM_WIDTH, FRAME_RESULT_ITEM_HEIGHT);

    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF FrameResultItem::boundingRect() const
{
    if(m_boundingRect.isNull())
        return QRectF(QPointF(0,0), QPointF(0,0));

    return m_boundingRect;
}

void FrameResultItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->fillRect(m_boundingRect, QColor(192, 192, 192));

    QRect imageRect = getFrameRect(QRect(0, 0, FRAME_RESULT_ITEM_WIDTH, FRAME_RESULT_ITEM_HEIGHT), m_frameResult.faceImage.size());
    painter->drawImage(imageRect, m_frameResult.faceImage);

    if(isSelected() && !m_frameResult.faceImage.isNull())
    {
        painter->save();
        painter->setOpacity(0.3);
        painter->fillRect(imageRect, Qt::blue);
        painter->restore();
    }

    painter->save();
    if(isSelected())
        painter->setPen(QPen(QColor(0, 255, 0), 2));
    else
        painter->setPen(QPen(QColor(100, 100, 100), 2));
    QRect boundingRect(m_boundingRect.left(), m_boundingRect.top(), m_boundingRect.width() - 1, m_boundingRect.height() - 1);
    painter->drawRect(boundingRect);

    painter->restore();
}

void FrameResultItem::setBoundingRect(QRect rect)
{
    m_boundingRect = rect;
}

void FrameResultItem::setInfo(FRAME_RESULT frameResult)
{
    m_frameResult = frameResult;
}


void FrameResultItem::setEmpty()
{
    m_frameResult.faceImage = QImage();
    m_frameResult.faceRect = QRect();
    m_frameResult.featData.resize(0);
}
