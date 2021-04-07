#ifndef NOKEYGRAPHICSVIEW_H
#define NOKEYGRAPHICSVIEW_H

#include <QGraphicsView>

class NoKeyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit NoKeyGraphicsView(QWidget *parent = 0);

signals:

public slots:

protected:
    void    keyPressEvent(QKeyEvent* e);

};

#endif // NOKEYGRAPHICSVIEW_H
