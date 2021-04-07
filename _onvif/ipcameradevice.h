#ifndef IPCAMERADEVICE_H
#define IPCAMERADEVICE_H

#include <QObject>

#include "devicemanagement.h"
#include "eventmanagement.h"
#include "mediamanagement.h"

class IpCameraDevice : public QObject
{
    Q_OBJECT
public:
    explicit IpCameraDevice(QObject *parent = 0);
    ~IpCameraDevice();

    void    setDeviceService(QString serviceStr, QString ipAddressStr, QString userNameStr, QString passwordStr);
    QString deviceService();
signals:
    void    findNewCamera(QHash<QString, QString> deviceInfos);
    void    findFinished(QObject*);

public slots:
    void    slotStartFind();

private:
    QString     m_deviceServiceStr;
    QString     m_ipAddressStr;
    QString     m_userNameStr;
    QString     m_passwordStr;
};

#endif // IPCAMERADEVICE_H
