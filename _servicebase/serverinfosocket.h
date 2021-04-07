#ifndef SERVERINFOSOCKET_H
#define SERVERINFOSOCKET_H

#include "servicebase.h"
#include <QThread>
#include <QMutex>

#include <winsock.h>
//#include <winsock2.h>

class CameraProcessEngine;
class ServerInfoSocket : public QThread
{
    Q_OBJECT
public:
    explicit ServerInfoSocket(QObject *parent = 0);

    void    setCameraProcessEngines(QVector<CameraProcessEngine*> cameraProcessEngines);

    void    startSocket(SOCKET, SOCKADDR_IN, int);
    void    stopSocket();

public slots:
    void    slotBlackInfoChanged();

signals:
    void    receiveIpCameraInfos(QVector<IpCameraInfo>);
    void    receiveSurveillanceSetting(IpCameraInfo surveillanceSetting, int chanelIndex);
    void    receiveIpCameraInfo(IpCameraInfo surveillanceSetting, int chanelIndex);

protected:
    void    run();
    void    sendIpCameraInfos();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;

    QVector<CameraProcessEngine*>   m_cameraProcessEngines;

    int             m_blackInfoChanged;
    QMutex          m_blackInfoMutex;
};

#endif // SERVERINFOSOCKET_H
