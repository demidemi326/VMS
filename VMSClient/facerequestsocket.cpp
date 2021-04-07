#include "facerequestsocket.h"
#include "clientbase.h"
#include "socketbase.h"
#include "serverinfosocket.h"
#include "base.h"

#include <QtWidgets>

#ifdef SEND_SCORE
float   g_faceScore = 0;
#endif

FaceRectReceiveSocket::FaceRectReceiveSocket(QObject *parent) :
    QThread(parent)
{
    m_chanelIndex = -1;
    m_serverInfoSocket = NULL;
}

void FaceRectReceiveSocket::setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex)
{
    if(m_serverInfoSocket)
    {
        disconnect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int,int,int)));

        m_serverInfoSocket = NULL;
    }

    if(serverInfoSocket)
    {
        m_serverInfoSocket = serverInfoSocket;
        m_chanelIndex = chanelIndex;

        connect(m_serverInfoSocket, SIGNAL(chanelStatusChanged(int,int,int)), this, SLOT(slotChanelStatusChanged(int,int,int)));
    }
}


void FaceRectReceiveSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void FaceRectReceiveSocket::slotReconnect()
{
    stopSocket();

    start();
}

void FaceRectReceiveSocket::slotChanelStatusChanged(int serverIndex, int chanel, int status)
{
    if(chanel != m_chanelIndex)
        return;

    if(status == CHANEL_STATUS_STOP)
        stopSocket();
    else
        slotReconnect();
}

void FaceRectReceiveSocket::run()
{
    if(m_chanelIndex < 0 || m_serverInfoSocket == NULL || m_chanelIndex >= m_serverInfoSocket->serverInfo().ipCameraInfos.size())
        return;

    SOCKADDR_IN target;
    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
        return;

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfoSocket->serverInfo().port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfoSocket->serverInfo().ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
        return;

    int ret = authenticateSocket();
    if(ret)
        recvFaceResult();

    closesocket(m_socket);
}


int FaceRectReceiveSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfoSocket->serverInfo().userName + "\n" + m_serverInfoSocket->serverInfo().password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "FACE_RESULT");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

void FaceRectReceiveSocket::recvFaceResult()
{
    while(1)
    {        
        QByteArray receiveData;

        int ret = receiveDataBySocket(m_socket, receiveData);
        if(ret <= 0)
            break;

        send(m_socket, (char*)&ret, sizeof(int), 0);

        qint64 imageTime, serverTime;
        QVector<QRect> faceResults;
        QVector<float> faceScores;

        QBuffer receiveDataBuffer(&receiveData);
        receiveDataBuffer.open(QIODevice::ReadOnly);

        QDataStream receiveDataStream(&receiveDataBuffer);

        receiveDataStream >> faceResults;
#ifdef SEND_SCORE
        receiveDataStream >> faceScores;
#endif
        receiveDataStream >> imageTime;
        receiveDataStream >> serverTime;

        QByteArray frameData;
        receiveDataStream >> frameData;

        receiveDataBuffer.close();

#ifdef SEND_SCORE
        if(faceScores.size())
            g_faceScore = faceScores[0];
#endif
        QImage frameImage = QImage::fromData(frameData, "JPG");

        emit receivedFaceResult(faceResults, imageTime, serverTime - QDateTime::currentDateTime().toMSecsSinceEpoch(), frameImage);
    }

    QVector<QRect> emptyResults;
    emit receivedFaceResult(emptyResults, 0, 0, QImage());
}
