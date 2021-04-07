#ifndef MONITORINGSERVEILLANCESETTINGDLG_H
#define MONITORINGSERVEILLANCESETTINGDLG_H

#include <QDialog>

#include "clientbase.h"

namespace Ui {
class MonitoringServeillanceSettingDlg;
}

class ServerInfoSocket;
class MonitoringServeillanceSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MonitoringServeillanceSettingDlg(QWidget *parent = 0);
    ~MonitoringServeillanceSettingDlg();

    void    setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex);

signals:
    void    valueChanged();

public slots:
    void    slotOk();

    void    slotFaceSizeClicked();
    void    slotThresoldClicked();
    void    slotBlackCandidateCountClicked();
    void    slotThresoldChanged(int value);
    void    slotBlackCandidateCountChanged(int value);
    void    slotMinFaceSizeChanged(int value);

    void    slotValueChanged();

private:
    void    retranslateUI();

private:
    Ui::MonitoringServeillanceSettingDlg *ui;

    ServerInfoSocket*   m_serverInfoSocket;
    int                 m_chanelIndex;

    IpCameraInfo        m_ipCameraInfo;
};

#endif // MONITORINGSERVEILLANCESETTINGDLG_H
