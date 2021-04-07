#ifndef HWNDWIDGET_H
#define HWNDWIDGET_H

#include <QtWidgets>

class HWNDWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HWNDWidget(QWidget* parent = 0);

protected:
    void    paintEvent(QPaintEvent* e);
};

#endif // HWNDWIDGET_H
