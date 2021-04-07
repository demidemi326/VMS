#ifndef CHANELDATASENDINGSOCKET_H
#define CHANELDATASENDINGSOCKET_H

#include <QtWidgets>
#include <winsock.h>
//#include <winsock2.h>

class ChanelDataSendingSocket : public QThread
{
    Q_OBJECT
public:
    explicit ChanelDataSendingSocket(QObject *parent = 0);

    void    startSocket(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void    stopSocket();

signals:
    void    socketError(QObject* obj);

public slots:

protected:
    virtual void run();

protected:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;
};

#endif // CHANELDATASENDINGSOCKET_H
