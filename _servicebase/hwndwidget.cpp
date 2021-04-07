#include "hwndwidget.h"

#include <QtWidgets>
#include <qwindowdefs.h>

HWNDWidget::HWNDWidget(QWidget* parent)
    : QWidget(parent)
{
}

void HWNDWidget::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    WId id = effectiveWinId();
    HWND hWnd = (HWND)id;

    HDC dc = ::GetDC(hWnd);
    ::LineTo(dc,20,20);
    ::ReleaseDC(hWnd,dc);
}
