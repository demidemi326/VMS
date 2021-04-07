#ifndef RESULTALARMDLG_H
#define RESULTALARMDLG_H

#include <QWidget>

#include "clientbase.h"

namespace Ui {
class ResultAlarmDlg;
}

class ResultAlarmDlg : public QWidget
{
    Q_OBJECT

public:
    explicit ResultAlarmDlg(QWidget *parent = 0);
    ~ResultAlarmDlg();

    void    setData(int alrmId, QVector<BLACK_RECOG_RESULT> blackRecogResult);
    void    retranslateUI();

    int     alarmID() {return m_alarmId;}

public slots:
    void    slotClose();

signals:
    void    closed();

protected:
    void    resizeEvent(QResizeEvent *);
    void    paintEvent(QPaintEvent *);
    void    updateFaceImages();
    void    closeEvent(QCloseEvent *);

private:
    Ui::ResultAlarmDlg *ui;

    QVector<BLACK_RECOG_RESULT> m_blackRecogResult;


    int m_alarmId;
};

#endif // RESULTALARMDLG_H
