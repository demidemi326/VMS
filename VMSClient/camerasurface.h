#ifndef CAMERASURFACE_H
#define CAMERASURFACE_H

#include <QWidget>
#include <QImage>
#include "ipcamera.h"

class CameraSurface : public QWidget
{
    Q_OBJECT
public:
    explicit CameraSurface(QWidget *parent = 0);
    ~CameraSurface();

    void    setImage(QImage image, int width, int height);

protected:
    void    paintEvent(QPaintEvent *);

protected:
    QImage      m_image;

    int         m_width;
    int         m_height;
};

#endif // CAMERASURFACE_H
