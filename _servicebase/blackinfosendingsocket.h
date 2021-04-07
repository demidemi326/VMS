#ifndef BLACKINFOSENDINGSOCKET_H
#define BLACKINFOSENDINGSOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class BlackInfoSendingSocket : public QThread
{
    Q_OBJECT
public:
    explicit BlackInfoSendingSocket(QObject *parent = 0);

    void    setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void    stopSocket();

    void    sendBlackInfo(PersonDatabase1 personDatabase, QVector<BlackPersonMetaInfo> blackMetaInfo);

signals:
    void    sendingFinished(QObject* obj);

public slots:

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;

    QVector<BlackPersonMetaInfo>    m_blackMetaInfo;
    PersonDatabase1                 m_personDatabase;
};

#endif // BLACKINFOSENDINGSOCKET_H
