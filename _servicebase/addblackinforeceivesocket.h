#ifndef ONEBLACKPERSONRECEIVESOCKET_H
#define ONEBLACKPERSONRECEIVESOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class AddBlackInfoReceiveSocket : public QThread
{
    Q_OBJECT
public:
    explicit AddBlackInfoReceiveSocket(QObject *parent = 0);

    void setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void stopSocket();
signals:
    void receiveFinished(QObject*);
    void receiveBlackPersoInfo(Person1 personFeatInfo, BlackPersonMetaInfo personMetaInfo);
    void receivedBlackPersonData(QByteArray personData);

public slots:

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;
};

#endif // ONEBLACKPERSONRECEIVESOCKET_H
