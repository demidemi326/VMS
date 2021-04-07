#ifndef SEARCHCAMERAITEM_H
#define SEARCHCAMERAITEM_H

#include <QObject>
#include <QGraphicsItem>

#define SEARCH_CAMERA_ITEM_WIDTH 220
#define SEARCH_CAMERA_ITEM_HEIGHT 80

class SearchCameraItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    SearchCameraItem();
    ~SearchCameraItem();

    QRectF  boundingRect() const;
    void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void    setBoundingRect(QRect rect);

    void    setInfo(QHash<QString, QString> deviceInfos);
    QHash<QString, QString>     info();

    bool    isEqual(QHash<QString, QString>);
private:
    QRect               m_boundingRect;

    QHash<QString, QString> m_deviceInfos;
};

#endif // SEARCHCAMERAITEM_H
