#ifndef IPCAMERAONESETTINGDLG_H
#define IPCAMERAONESETTINGDLG_H

#include "clientbase.h"
#include <QDialog>

namespace Ui {
class IpCameraOneSettingDlg;
}

class IpCameraOneSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit IpCameraOneSettingDlg(QWidget *parent = 0);
    ~IpCameraOneSettingDlg();

    void    setIpCameraInfo(IpCameraInfo ipCameraInfo);
    IpCameraInfo    ipCameraInfo();

public slots:
    void    slotOk();
    void    slotCancel();

    void    slotMinFaceSizeChanged(int value);
    void    slotThresoldChanged(int value);

    void    slotFaceSizeClicked();
    void    slotThresoldClicked();

private:
    Ui::IpCameraOneSettingDlg *ui;

    IpCameraInfo        m_ipCameraInfo;
};

#endif // IPCAMERAONESETTINGDLG_H
