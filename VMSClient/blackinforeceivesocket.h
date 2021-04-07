#ifndef BLACKINFORECEIVESOCKET_H
#define BLACKINFORECEIVESOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class BlackInfoReceiveSocket : public QThread
{
    Q_OBJECT
public:
    explicit BlackInfoReceiveSocket(QObject *parent = 0);

    void    startSocket(ServerInfo serverInfo, int chanelIndex);
    void    stopSocket();

signals:
    void    receivedBlackInfos(QVector<BLACK_PERSON> blackInfos);

protected:
    void    run();

    int     authenticateSocket();
    void    recvBlackInfo();
private:
    SOCKET      m_socket;

    ServerInfo              m_serverInfo;
    int                     m_chanelIndex;
};

#endif // BLACKINFORECEIVESOCKET_H
