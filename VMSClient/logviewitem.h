#ifndef LOGVIEWITEM_H
#define LOGVIEWITEM_H

#include "clientbase.h"

#include <QObject>
#include <QGraphicsItem>

#define LOG_ITEM_WIDTH 276
#define LOG_ITEM_IMAGE_HEIGHT 100
#define LOG_TEXT_HEIGHT 18
#define LOG_ITEM_HEIGHT LOG_ITEM_IMAGE_HEIGHT + LOG_TEXT_HEIGHT * 1

#define LOG_IMAGE_WIDTH 60
#define LOG_IMAGE_HEIGHT 75

#define LOG_CAND_IMAGE_WIDTH 60
#define LOG_CAND_IMAGE_HEIGHT 75

class LogViewItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit LogViewItem(QObject *parent = 0);

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

    LOG_RESULT       m_faceImageInfo;
};

#endif // LOGVIEWITEM_H
