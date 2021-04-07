#include "searchcameraitem.h"
#include "stringtable.h"

#include <QtWidgets>

#define TEXT_LINE_HEIGHT 20

SearchCameraItem::SearchCameraItem()
{
    m_boundingRect = QRect(0, 0, SEARCH_CAMERA_ITEM_WIDTH, SEARCH_CAMERA_ITEM_HEIGHT);

    setFlag(QGraphicsItem::ItemIsSelectable);
}

SearchCameraItem::~SearchCameraItem()
{

}

QRectF SearchCameraItem::boundingRect() const
{
    if(m_boundingRect.isNull())
        return QRectF(QPointF(0,0), QPointF(0,0));

    return m_boundingRect;
}

void SearchCameraItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);

    QRect boundingRect = m_boundingRect;
    //boundingRect.moveTo(pos().toPoint());

    painter->setPen(QColor(128, 128, 128));
    if(isSelected())
        painter->setBrush(QColor(61, 61, 61));
    else
        painter->setBrush(QColor(205, 205, 205));

    painter->drawRoundedRect(boundingRect, 10, 10);

    if(isSelected())
        painter->setPen(Qt::white);
    else
        painter->setPen(Qt::black);

    QFont font = painter->font();
    QFont bFont = font;
    bFont.setBold(true);

    painter->setFont(bFont);

    QRect leftTextRect(boundingRect.left() + 7, boundingRect.top() + 10, boundingRect.width(), boundingRect.height());

    if(isSelected())
        painter->drawPixmap(QRect(boundingRect.left() + 170, boundingRect.top() + 5, TEXT_LINE_HEIGHT, TEXT_LINE_HEIGHT), QPixmap(":/images/check.png"));

    painter->drawText(leftTextRect, Qt::AlignTop | Qt::AlignLeft, m_deviceInfos["mf"]);
    leftTextRect.setTop(leftTextRect.top() + TEXT_LINE_HEIGHT + 7);

    painter->drawText(leftTextRect, Qt::AlignTop | Qt::AlignLeft, StringTable::Str_Address + ":");
    leftTextRect.setTop(leftTextRect.top() + TEXT_LINE_HEIGHT);

    painter->drawText(leftTextRect, Qt::AlignTop | Qt::AlignLeft, StringTable::Str_VideoSource + ":");

    painter->setFont(font);
    QRect rightTextRect(boundingRect.left() + 95, boundingRect.top() + 10, boundingRect.width(), boundingRect.height());
    rightTextRect.setTop(rightTextRect.top() + TEXT_LINE_HEIGHT + 7);
    painter->drawText(rightTextRect, Qt::AlignTop | Qt::AlignLeft, m_deviceInfos["ip"]);
    rightTextRect.setTop(rightTextRect.top() + TEXT_LINE_HEIGHT);
    painter->drawText(rightTextRect, Qt::AlignTop | Qt::AlignLeft, m_deviceInfos["videosource"]);

    painter->restore();
}

void SearchCameraItem::setBoundingRect(QRect rect)
{
    m_boundingRect = rect;
}

void SearchCameraItem::setInfo(QHash<QString, QString> deviceInfos)
{
    m_deviceInfos = deviceInfos;
}

QHash<QString, QString> SearchCameraItem::info()
{
    return m_deviceInfos;
}

bool SearchCameraItem::isEqual(QHash<QString, QString> deviceInfos)
{
    if(m_deviceInfos["ip"] == deviceInfos["ip"] &&
            m_deviceInfos["videosource"] == deviceInfos["videosource"])
        return true;

    return false;
}
