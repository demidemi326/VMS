#include "nextbuttonitem.h"

#include <QtWidgets>

NextButtonItem::NextButtonItem(QObject *)
{
    m_boundingRect = QRect(0, 0, NEXT_BUTTON_ITEM_WIDTH, NEXT_BUTTON_ITEM_HEIGHT);
    m_mouseState = 0;
}

QRectF NextButtonItem::boundingRect() const
{
    if(m_boundingRect.isNull())
        return QRectF(QPointF(0,0), QPointF(0,0));

    return m_boundingRect;
}

void NextButtonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if(m_mouseState == 0)
        painter->fillRect(m_boundingRect, QColor(150, 150, 150));
    else
        painter->fillRect(m_boundingRect, QColor(120, 120, 120));

    QRect imageRect = getFrameRect(QRect(0, 0, NEXT_BUTTON_ITEM_WIDTH, NEXT_BUTTON_ITEM_HEIGHT), m_pixmap.size());
    painter->drawPixmap(imageRect, m_pixmap);
}

void NextButtonItem::setBoundingRect(QRect rect)
{
    m_boundingRect = rect;
}


void NextButtonItem::setPixmap(QPixmap pixmap)
{
    m_pixmap = pixmap;
}

void NextButtonItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem::mousePressEvent(e);
    e->accept();
    m_mouseState = 1;
    update();
}

void NextButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsItem::mousePressEvent(e);
    m_mouseState = 0;
    update();

    emit clicked();
}
