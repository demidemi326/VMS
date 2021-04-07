#ifndef SELECTCAMERADLG_H
#define SELECTCAMERADLG_H

#include <QDialog>
#include "devicesearcher.h"
#include "ipcamera.h"

namespace Ui {
class SelectCameraDlg;
}

using namespace ONVIF;

class IpCameraDevice;
class QGraphicsScene;

class SelectCameraDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SelectCameraDlg(QWidget *parent = 0);
    ~SelectCameraDlg();

    QHash<QString, QString>         cameraInfos();

public slots:
    void    slotOk();
    void    slotCancel();
    void    slotRefresh();
    void    slotReceiveDeviceData(QHash<QString,QString> receiveData);

    void    slotFindNewCamera(QHash<QString, QString> deviceInfos);
    void    slotCameraSelectionChanged();
    void    slotFrameChanged();

private:
    Ui::SelectCameraDlg *ui;

    DeviceSearcher*     m_deviceSearcher;
    QVector<IpCameraDevice*>      m_ipCameraDevices;
    QStringList         m_cameraSevices;

    QGraphicsScene*     m_scene;

    IpCamera*           m_ipCamera;
};

#endif // SELECTCAMERADLG_H
