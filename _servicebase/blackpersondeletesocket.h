#ifndef BLACKPERSONDELETESOCKET_H
#define BLACKPERSONDELETESOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>


class BlackPersonDeleteSocket : public QThread
{
    Q_OBJECT
public:
    explicit BlackPersonDeleteSocket(QObject *parent = 0);

    void setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void stopSocket();

signals:
    void receivedDeleteInfoFinished(QObject*);
    void receiveDeleteBlackPersonInfo(QStringList deleteNames);

public slots:

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;
};

#endif // BLACKPERSONDELETESOCKET_H
