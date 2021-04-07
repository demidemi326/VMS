#ifndef FRAMERESULTITEM_H
#define FRAMERESULTITEM_H

#include "clientbase.h"

#include <QObject>
#include <QGraphicsItem>

#define FRAME_RESULT_ITEM_WIDTH 80
#define FRAME_RESULT_ITEM_HEIGHT 100
#define FRAME_RESULT_ITEM_GAP 5

class FrameResultItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit FrameResultItem(QObject *parent = 0);

    QRectF  boundingRect() const;
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void    setBoundingRect(QRect rect);

    void    setInfo(FRAME_RESULT frameResult);
    void    setEmpty();
signals:

public slots:

private:
    QRect               m_boundingRect;

    FRAME_RESULT        m_frameResult;
};

#endif // FRAMERESULTITEM_H
