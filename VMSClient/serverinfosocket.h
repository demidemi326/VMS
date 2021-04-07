#ifndef SERVERINFOSOCKET_H
#define SERVERINFOSOCKET_H

#include "clientbase.h"

#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

#define SERVER_STOP 0
#define SERVER_START 1
#define SERVER_NONE 2
#define SERVER_SETTING_IPCAMERA 3
#define SERVER_SETTING_SURVEILLANCE 4
#define SERVER_SETTING_ONE_IPCAMERA 5

#define CHANEL_STATUS_STOP 0
#define CHANEL_STATUS_RUNNING 1

class ServerInfoSocket : public QThread
{
    Q_OBJECT
public:
    explicit ServerInfoSocket(QObject *parent = 0);

    void    startSocket(ServerInfo serverInfo, int serverIndex);
    void    stopSocket();
    void    changeServerInfo(ServerInfo serverInfo);

    void    setIpCameraInfos(QVector<IpCameraInfo> ipCameraInfos);
    void    setSurveillanceSetting(IpCameraInfo ipCameraInfo, int chanelIndex);
    void    setIpCameraInfo(IpCameraInfo ipCameraInfo, int chanelIndex);

    ServerInfo  serverInfo();
    int         status();
    void        setStatus(int status, int sendStatus = 0);

    int         getChanelStatus(int chanelIndex);
    qint64  diffTime();

signals:
    void    statusChanged(int serverIndex, int status);
    void    chanelStatusChanged(int serverIndex, int chanel, int status);
    void    blackInfoChanged(int serverIndex, int chanel);


protected:
    void    run();

    int     authenticateSocket();
    void    receiveServerInfo();

private:
    SOCKET      m_socket;
    int         m_status;
    QMutex      m_mutex;
    QMutex      m_cameraMutex;
    QMutex      m_cameraStatusMutex;

    ServerInfo      m_serverInfo;
    int             m_serverIndex;
    QByteArray      m_cameraStatus;

    IpCameraInfo    m_tmpCameraInfo;
    int             m_tmpChanelIndex;

    qint64          m_diffTime;
};

#endif // SERVERINFOSOCKET_H
