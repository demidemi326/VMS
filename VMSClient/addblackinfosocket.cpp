#include "addblackinfosocket.h"
#include "socketbase.h"

#include <QtWidgets>

AddBlackInfoSocket::AddBlackInfoSocket(QObject *parent) :
    QThread(parent)
{
}

void AddBlackInfoSocket::setInfo(ServerInfo serverInfo, int chanelIndex, QVector<BLACK_PERSON> blackPersonInfos)
{
    m_serverInfo = serverInfo;
    m_chanelIndex = chanelIndex;
    m_blackPersonInfos = blackPersonInfos;
}

void AddBlackInfoSocket::startSocket()
{
    stopSocket();
    start();
}

void AddBlackInfoSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void AddBlackInfoSocket::run()
{
    SOCKADDR_IN target; //Information about host

    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
    {
        enrollFinished(this);
        return;
    }

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfo.port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
    {
        enrollFinished(this);
        return;
    }

    int ret = authenticateSocket();
    if(ret)
        sendBlackPersonData();

    closesocket(m_socket);

    enrollFinished(this);
}

int AddBlackInfoSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "ONE_BLACK");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

int AddBlackInfoSocket::sendBlackPersonData()
{
    QVector<BLACK_PERSON> blackPersonInfos;

    for(int i = 0; i < m_blackPersonInfos.size(); i ++)
    {
        if(m_blackPersonInfos[i].featData.size())
            blackPersonInfos.append(m_blackPersonInfos[i]);
    }

    if(blackPersonInfos.size() == 0)
        return 0;

    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << blackPersonInfos.size();

    for(int i = 0; i < blackPersonInfos.size(); i ++)
    {
        sendingDataStream << blackPersonInfos[i].featData;
        sendingDataStream << blackPersonInfos[i].name;
        sendingDataStream << blackPersonInfos[i].gender;
        sendingDataStream << blackPersonInfos[i].birthDay;
        sendingDataStream << blackPersonInfos[i].address;
        sendingDataStream << blackPersonInfos[i].description;
        sendingDataStream << blackPersonInfos[i].personType;
        sendingDataStream << blackPersonInfos[i].faceData;
    }

    sendingBuffer.close();

    return sendDataBySocket(m_socket, sendingData);
}
