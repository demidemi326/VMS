#ifndef CONTROLBUTTON_H
#define CONTROLBUTTON_H

#include <QLabel>

#define BTN_STATUS_NORMAL 0
#define BTN_STATUS_UP 1
#define BTN_STATUS_DOWN 2


class ControlButton : public QLabel
{
    Q_OBJECT
public:
    explicit ControlButton(QWidget *parent = 0);
    ~ControlButton();

    void setButtonPix(QPixmap normalPix, QPixmap upPix, QPixmap downPix);

signals:
    void clicked();

public slots:

protected:
    void    mousePressEvent(QMouseEvent*);
    void    mouseMoveEvent(QMouseEvent*);
    void    mouseReleaseEvent(QMouseEvent*);
    void    leaveEvent(QEvent*);

    void    updatePix();

private:
    QPixmap m_normalPix;
    QPixmap m_upPix;
    QPixmap m_downPix;

    int     m_state;
    int     m_mouseState;
};

#endif // CONTROLBUTTON_H
