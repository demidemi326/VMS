#include "blackpersondeletesocket.h"
#include "socketbase.h"

#include <QtWidgets>

BlackPersonDeleteSocket::BlackPersonDeleteSocket(QObject *parent) :
    QThread(parent)
{
}

void BlackPersonDeleteSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;

    start();
}

void BlackPersonDeleteSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void BlackPersonDeleteSocket::run()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
    {
        emit receivedDeleteInfoFinished(this);
        return;
    }

    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    QDataStream receiveDataStream(&receiveDataBuffer);

    int deleteNameCount = 0;
    receiveDataStream >> deleteNameCount;

    QStringList deleteNames;
    for(int i = 0; i < deleteNameCount; i ++)
    {
        QString deleteName;
        receiveDataStream >> deleteName;

        deleteNames.append(deleteName);
    }

    receiveDataBuffer.close();

    emit receiveDeleteBlackPersonInfo(deleteNames);
    emit receivedDeleteInfoFinished(this);
}
