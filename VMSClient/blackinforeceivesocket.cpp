#include "blackinforeceivesocket.h"
#include "socketbase.h"

#include <QtWidgets>

BlackInfoReceiveSocket::BlackInfoReceiveSocket(QObject *parent) :
    QThread(parent)
{
    m_chanelIndex = -1;
}

void BlackInfoReceiveSocket::startSocket(ServerInfo serverInfo, int chanelIndex)
{
    m_serverInfo = serverInfo;
    m_chanelIndex = chanelIndex;

    start();
}

void BlackInfoReceiveSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void BlackInfoReceiveSocket::run()
{
    SOCKADDR_IN target; //Information about host

    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
        return;

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfo.port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
    { // error
        return;
    }

    int ret = authenticateSocket();
    if(ret)
        recvBlackInfo();

    closesocket(m_socket);
}

int BlackInfoReceiveSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "BLACK_INFO_RECEIVE");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

void BlackInfoReceiveSocket::recvBlackInfo()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
        return;

    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    int receiveInfoSize = 0;
    QVector<BLACK_PERSON> blackPersonInfos;

    QDataStream receiveDataStream(&receiveDataBuffer);

    receiveDataStream >> receiveInfoSize;
    for(int i = 0; i < receiveInfoSize; i ++)
    {
        BLACK_PERSON personInfo;
        receiveDataStream >> personInfo.featData;
        receiveDataStream >> personInfo.name;
        receiveDataStream >> personInfo.gender;
        receiveDataStream >> personInfo.birthDay;
        receiveDataStream >> personInfo.address;
        receiveDataStream >> personInfo.description;
        receiveDataStream >> personInfo.personType;
        receiveDataStream >> personInfo.faceData;

        blackPersonInfos.append(personInfo);
    }

    receiveDataBuffer.close();

    emit receivedBlackInfos(blackPersonInfos);
}
