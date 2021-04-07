#ifndef ADDONEBLACKLISTSOCKET_H
#define ADDONEBLACKLISTSOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class AddBlackInfoSocket : public QThread
{
    Q_OBJECT
public:
    explicit AddBlackInfoSocket(QObject *parent = 0);

    void    setInfo(ServerInfo serverInfo, int chanelIndex, QVector<BLACK_PERSON> blackPersonInfos);
    void    startSocket();
    void    stopSocket();

signals:
    void    enrollFinished(QObject*);

public slots:

protected:
    void    run();

    int     authenticateSocket();
    int     sendBlackPersonData();
private:
    SOCKET      m_socket;

    ServerInfo  m_serverInfo;
    int         m_chanelIndex;

    QVector<BLACK_PERSON>    m_blackPersonInfos;
};

#endif // ADDONEBLACKLISTSOCKET_H
