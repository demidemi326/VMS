#ifndef CLIENTLISTENINGSOCKET_H
#define CLIENTLISTENINGSOCKET_H

#include <QtWidgets>
#include <winsock.h>
//#include <winsock2.h>


class ClientListeningSocket : public QThread
{
    Q_OBJECT
public:
    explicit ClientListeningSocket(QObject *parent = 0);

    void    startListening(int port);
    void    stopListening();

signals:
    void    newConnection(SOCKET, SOCKADDR_IN, int);

public slots:

protected:
    void    run();

    SOCKET  m_serverSocket;
    int     m_listeningPort;
    int     m_running;
};

#endif // CLIENTLISTENINGSOCKET_H
