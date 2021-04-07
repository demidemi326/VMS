#include "chaneldatasendingsocket.h"

#include <QtWidgets>

ChanelDataSendingSocket::ChanelDataSendingSocket(QObject *parent) :
    QThread(parent)
{
}

void ChanelDataSendingSocket::startSocket(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;

    start();
}

void ChanelDataSendingSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void ChanelDataSendingSocket::run()
{

}
