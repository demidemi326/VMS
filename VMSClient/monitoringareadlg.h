#ifndef MONITORINGAREADLG_H
#define MONITORINGAREADLG_H

#include <QDialog>

#include "clientbase.h"

namespace Ui {
class MonitoringAreaDlg;
}

class MonitoringAreaDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MonitoringAreaDlg(QWidget *parent = 0);
    ~MonitoringAreaDlg();

    void    setMonitoringAreaInfos(QVector<MonitoringAreaInfo>, int editIndex = -1);
    MonitoringAreaInfo monitoringAreaInfo();

public slots:
    void    slotOk();
    void    slotCancel();

private:
    void retranslateUI();

private:
    Ui::MonitoringAreaDlg *ui;

    QVector<MonitoringAreaInfo> m_monitoringAreaInfos;
    int     m_editIndex;
};

#endif // MONITORINGAREADLG_H
