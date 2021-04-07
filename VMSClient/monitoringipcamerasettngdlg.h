#ifndef MONITORINGIPCAMERASETTNGDLG_H
#define MONITORINGIPCAMERASETTNGDLG_H

#include "clientbase.h"

#include <QDialog>

namespace Ui {
class MonitoringIpCameraSettngDlg;
}

class ServerInfoSocket;
class MonitoringIpCameraSettngDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MonitoringIpCameraSettngDlg(QWidget *parent = 0);
    ~MonitoringIpCameraSettngDlg();

    void    setChanelInfo(ServerInfo serverInfo, int chanelIndex);
    IpCameraInfo    ipCameraInfo();

public slots:
    void    slotOk();
    void    slotCancel();

    void    slotSelectCamera();
    void    slotClear();
private:
    void    retranslateUI();

private:
    Ui::MonitoringIpCameraSettngDlg *ui;
    IpCameraInfo        m_ipCameraInfo;

    ServerInfo          m_serverInfo;
    int                 m_chanelIndex;
};

#endif // MONITORINGIPCAMERASETTNGDLG_H
