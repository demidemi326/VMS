#ifndef BLACKRESULTVIEW_H
#define BLACKRESULTVIEW_H

#include <QWidget>
#include "clientbase.h"

namespace Ui {
class BlackResultView;
}

class BlackResultView : public QWidget
{
    Q_OBJECT

public:
    explicit BlackResultView(QWidget *parent = 0);
    ~BlackResultView();

    void    setBlackResult(QVector<BLACK_RECOG_RESULT> blackResult);

protected:
    void    paintEvent(QPaintEvent *);

private:
    Ui::BlackResultView *ui;


    QVector<BLACK_RECOG_RESULT>     m_blackResult;

    QImage                          m_galleryImage;
    QImage                          m_probeImage;
};

#endif // BLACKRESULTVIEW_H
