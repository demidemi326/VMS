#ifndef ONEBLACKPERSONEDITSOCKET_H
#define ONEBLACKPERSONEDITSOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class OneBlackPersonEditSocket : public QThread
{
    Q_OBJECT
public:
    explicit OneBlackPersonEditSocket(QObject *parent = 0);

    void setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void stopSocket();

signals:
    void receiveEditInfoFinished(QObject*);
    void receiveEditBlackPersonInfo(QString oldName, Person1 personFeat, BlackPersonMetaInfo personMetaInfo);

public slots:

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;
};

#endif // ONEBLACKPERSONEDITSOCKET_H
