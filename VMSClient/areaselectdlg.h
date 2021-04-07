#ifndef AREASELECTDLG_H
#define AREASELECTDLG_H

#include "clientbase.h"
#include <QDialog>

namespace Ui {
class AreaSelectDlg;
}

class AreaSelectDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AreaSelectDlg(QWidget *parent = 0);
    ~AreaSelectDlg();

    void    setAreaInfo(QVector<MonitoringAreaInfo>);
    int     selectedIndex();

public slots:
    void    slotOk();
    void    slotCancel();
    void    retranslateUI();

private:
    Ui::AreaSelectDlg *ui;

    QVector<MonitoringAreaInfo> m_areaInfo;
};

#endif // AREASELECTDLG_H
