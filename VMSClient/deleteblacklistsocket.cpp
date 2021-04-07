#include "deleteblacklistsocket.h"
#include "socketbase.h"

#include <QtWidgets>


DeleteBlackListSocket::DeleteBlackListSocket(QObject *parent) :
    QThread(parent)
{
}

void DeleteBlackListSocket::setInfo(ServerInfo serverInfo, int chanelIndex, QStringList deleteNames)
{
    m_serverInfo = serverInfo;
    m_chanelIndex = chanelIndex;
    m_deleteNames = deleteNames;
}

void DeleteBlackListSocket::startDelete()
{
    stopDelete();
    start();
}

void DeleteBlackListSocket::stopDelete()
{
    closesocket(m_socket);
    wait();
}

void DeleteBlackListSocket::run()
{
    SOCKADDR_IN target; //Information about host

    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
    {
        deleteFinished(this);
        return;
    }

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfo.port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
    { // error
        deleteFinished(this);
        return;
    }

    int ret = authenticateSocket();
    if(ret)
        sendDeleteBlackPersonData();

    closesocket(m_socket);

    deleteFinished(this);
}

int DeleteBlackListSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "DELETE_BLACK_INFO");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

int DeleteBlackListSocket::sendDeleteBlackPersonData()
{
    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << m_deleteNames.size();
    for(int i = 0; i < m_deleteNames.size(); i ++)
        sendingDataStream << m_deleteNames[i];

    sendingBuffer.close();

    return sendDataBySocket(m_socket, sendingData);
}
