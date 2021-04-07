#include "controlbutton.h"

#include <QtWidgets>

ControlButton::ControlButton(QWidget *parent) : QLabel(parent)
{
    setMouseTracking(true);
    m_state = BTN_STATUS_NORMAL;
    m_mouseState = 0;
}

ControlButton::~ControlButton()
{

}

void ControlButton::setButtonPix(QPixmap normalPix, QPixmap upPix, QPixmap downPix)
{
    m_normalPix = normalPix;
    m_upPix = upPix;
    m_downPix = downPix;

    updatePix();
}

void ControlButton::mousePressEvent(QMouseEvent* e)
{
    QLabel::mousePressEvent(e);
    if(rect().contains(e->pos()))
        m_state = BTN_STATUS_DOWN;

    m_mouseState = 1;

    updatePix();
}

void ControlButton::mouseMoveEvent(QMouseEvent* e)
{
    QLabel::mouseMoveEvent(e);

    if(rect().contains(e->pos()) && m_mouseState == 1)
        m_state = BTN_STATUS_DOWN;
    else if(rect().contains(e->pos()) && m_mouseState == 0)
        m_state = BTN_STATUS_UP;
    else
        m_state = BTN_STATUS_NORMAL;

    updatePix();
}

void ControlButton::mouseReleaseEvent(QMouseEvent* e)
{
    QLabel::mouseReleaseEvent(e);

    m_mouseState = 0;
    if(rect().contains(e->pos()))
    {
        m_state = BTN_STATUS_UP;
        emit clicked();
    }
    else
        m_state = BTN_STATUS_NORMAL;

    updatePix();
}

void ControlButton::leaveEvent(QEvent* e)
{
    QLabel::leaveEvent(e);
    m_state = BTN_STATUS_NORMAL;

    updatePix();
}

void ControlButton::updatePix()
{
    if(m_state == BTN_STATUS_NORMAL)
        setPixmap(m_normalPix);
    else if(m_state == BTN_STATUS_DOWN)
        setPixmap(m_downPix);
    else
        setPixmap(m_upPix);
}
