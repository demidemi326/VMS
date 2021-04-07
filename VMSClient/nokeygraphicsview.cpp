#include "nokeygraphicsview.h"

#include <QtWidgets>

NoKeyGraphicsView::NoKeyGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

void NoKeyGraphicsView::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}
