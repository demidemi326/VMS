#ifndef CACHELOGVIEWITEM_H
#define CACHELOGVIEWITEM_H

#include "clientbase.h"

#include <QObject>
#include <QGraphicsItem>

#define CACHE_LOG_ITEM_WIDTH 80
#define CACHE_LOG_TEXT_HEIGHT 18
#define CACHE_LOG_ITEM_HEIGHT CACHE_LOG_ITEM_WIDTH + CACHE_LOG_TEXT_HEIGHT * 3

class CacheLogViewItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit CacheLogViewItem(QObject *parent = 0);

    QRectF  boundingRect() const;
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void    setBoundingRect(QRect rect);

    void    setInfo(LOG_RESULT logResult);
signals:
    void    logItemDoubleClicked(LOG_RESULT);

public slots:

protected:
    void    mouseDoubleClickEvent(QGraphicsSceneMouseEvent* );

private:
    QRect               m_boundingRect;

    LOG_RESULT          m_logResult;
};


#endif // CACHELOGVIEWITEM_H
