#ifndef NEXTBUTTONITEM_H
#define NEXTBUTTONITEM_H

#include "clientbase.h"
#include <QObject>
#include <QGraphicsItem>
#include <QPixmap>

#define NEXT_BUTTON_ITEM_WIDTH 20
#define NEXT_BUTTON_ITEM_HEIGHT 100

class NextButtonItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit NextButtonItem(QObject *parent = 0);

    QRectF  boundingRect() const;
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void    setBoundingRect(QRect rect);

    void    setPixmap(QPixmap);
signals:
    void    clicked();

public slots:

protected:
    void                mousePressEvent(QGraphicsSceneMouseEvent*);
    void                mouseReleaseEvent(QGraphicsSceneMouseEvent*);

private:
    QRect               m_boundingRect;
    int                 m_mouseState;

    QPixmap             m_pixmap;
};

#endif // NEXTBUTTONITEM_H
