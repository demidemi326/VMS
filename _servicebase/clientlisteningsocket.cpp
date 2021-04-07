#include "clientlisteningsocket.h"
#include "servicebase.h"

#include <QtWidgets>

ClientListeningSocket::ClientListeningSocket(QObject *parent) :
    QThread(parent)
{
    m_running = 0;
}

void ClientListeningSocket::startListening(int port)
{
    m_listeningPort = port;
    m_running = 1;
    start();
}

void ClientListeningSocket::stopListening()
{
    m_running = 0;
    closesocket(m_serverSocket);
    m_serverSocket = INVALID_SOCKET;
    wait();
}

void ClientListeningSocket::run()
{
    SOCKADDR_IN addr; // the address structure for a TCP socket

    addr.sin_family = AF_INET;      // Address family Internet
    addr.sin_port = htons (m_listeningPort);   // Assign port to this socket
    addr.sin_addr.s_addr = htonl (INADDR_ANY);   // No destination
    //addr.sin_addr.s_addr = inet_addr ("192.168.1.64");   // No destination

    m_serverSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket

    if (m_serverSocket == INVALID_SOCKET)
    {
        return;
    }

    if (bind(m_serverSocket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR) //Try binding
    { // error
        return;
    }

    listen(m_serverSocket, 10); //Start listening

    while(m_running)
    {
        SOCKADDR_IN clientSocketIn;
        int clientSocketInLen = sizeof(SOCKADDR_IN);

        SOCKET clientSocket = accept(m_serverSocket, (struct sockaddr*)&clientSocketIn, &clientSocketInLen);

        if(clientSocket == INVALID_SOCKET || m_running == 0)
            break;

        emit newConnection(clientSocket, clientSocketIn, clientSocketInLen);
    }

    closesocket(m_serverSocket);
}
