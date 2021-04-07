#include "editoneblacklistsocket.h"
#include "socketbase.h"

#include <QtWidgets>

EditOneBlackListSocket::EditOneBlackListSocket(QObject *parent) :
    QThread(parent)
{
}

void EditOneBlackListSocket::setInfo(ServerInfo serverInfo, int chanelIndex, BLACK_PERSON blackPerson, QString oldName)
{
    m_serverInfo = serverInfo;
    m_chanelIndex = chanelIndex;
    m_blackPerson = blackPerson;
    m_oldName = oldName;
}

void EditOneBlackListSocket::startSocket()
{
    stopSocket();
    start();
}

void EditOneBlackListSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void EditOneBlackListSocket::run()
{
    SOCKADDR_IN target; //Information about host

    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
    {
        editFinished(this);
        return;
    }

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfo.port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
    { // error
        editFinished(this);
        return;
    }

    int ret = authenticateSocket();
    if(ret)
        sendBlackPersonData();

    closesocket(m_socket);

    editFinished(this);
}

int EditOneBlackListSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "EDIT_ONE_BLACK");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

int EditOneBlackListSocket::sendBlackPersonData()
{
    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << m_oldName;
    sendingDataStream << m_blackPerson.name;
    sendingDataStream << m_blackPerson.gender;
    sendingDataStream << m_blackPerson.birthDay;
    sendingDataStream << m_blackPerson.address;
    sendingDataStream << m_blackPerson.description;
    sendingDataStream << m_blackPerson.personType;
    sendingDataStream << m_blackPerson.faceData;
    sendingDataStream << m_blackPerson.featData;

    sendingBuffer.close();

    return sendDataBySocket(m_socket, sendingData);
}
