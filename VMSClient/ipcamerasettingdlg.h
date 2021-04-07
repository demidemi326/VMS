#ifndef IPCAMERASETTINGDLG_H
#define IPCAMERASETTINGDLG_H

#include <QDialog>

#include "clientbase.h"

namespace Ui {
class IpCameraSettingDlg;
}

class QStandardItem;
class QStandardItemModel;
class IpCameraSettingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit IpCameraSettingDlg(QWidget *parent = 0);
    ~IpCameraSettingDlg();

    void setServerInfo(ServerInfo serverInfo);

    QVector<IpCameraInfo>  ipCameraInfos();

public slots:
    void    slotIPCameraSelectionChanged(const QItemSelection&, const QItemSelection&);
    void    slotApply();
    void    slotChangedIpCameraSetting(QString);

    void    slotMinFaceSizeChanged(int value);
    void    slotThresoldChanged(int value);
    void    slotBlackCandidateCountChanged(int value);
    void    slotFaceSizeClicked();
    void    slotThresoldClicked();
    void    slotBlackCandidateCountClicked();

    void    slotClear();

    void    slotSelectCamera();

private:
    void    changeSelectItem();
    void    selectIpCamera(int index);
    void    retranslateUI();

private:
    Ui::IpCameraSettingDlg *ui;

    ServerInfo                 m_serverInfo;

    QStandardItemModel*        m_model;
    QItemSelectionModel*       m_selectionModel;

    QStandardItem*             m_rootItem;

    int                        m_selectedIndex;
};

#endif // IPCAMERASETTINGDLG_H
