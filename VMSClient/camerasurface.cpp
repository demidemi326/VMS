#include "camerasurface.h"
#include "clientbase.h"

#include <QtWidgets>

CameraSurface::CameraSurface(QWidget *parent) : QWidget(parent)
{
}

CameraSurface::~CameraSurface()
{
}

void CameraSurface::setImage(QImage image, int width, int height)
{
    m_image = image;
    m_width = width;
    m_height = height;
    update();
}

void CameraSurface::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);

    painter.fillRect(rect(), Qt::black);
    if(!m_image.isNull())
    {
        QFont font = painter.font();
        font.setPointSize(15);

        painter.setPen(Qt::green);
        painter.setFont(font);
        QString resolution;
        resolution.sprintf("%d * %d", m_width, m_height);
        painter.drawText(QRect(10, 10, rect().width(), rect().height()), Qt::AlignLeft | Qt::AlignTop, resolution);

        QRect frameRect = getFrameRect(rect(), m_image.size());
        painter.drawImage(frameRect, m_image);
    }

    painter.end();
}

