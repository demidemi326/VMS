#include "facesendingsocket.h"
#include "socketbase.h"

#include <QtWidgets>

FaceSendingSocket::FaceSendingSocket(QObject *parent) :
    QThread(parent)
{
    m_socket = INVALID_SOCKET;
    m_chanelIndex = -1;
}

void FaceSendingSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;
}

void FaceSendingSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void FaceSendingSocket::slotSendFaceResults(QVector<QRect> faceResults, qint64 imageTime)
{
    if(isRunning())
        return;

    m_faceResults = faceResults;
    m_imageTime = imageTime;

    start();
}

void FaceSendingSocket::run()
{
    QByteArray sendData;
    QBuffer sendingBuffer(&sendData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << m_faceResults;
    sendingDataStream << m_imageTime;
    sendingDataStream << QDateTime::currentDateTime().toMSecsSinceEpoch();

    sendingBuffer.close();

    int ret = send(m_socket, sendData.data(), sendData.size(), 0);
    if(ret <= 0)
        emit socketError(this);

    recv(m_socket, (char*)&ret, sizeof(int), 0);
}

