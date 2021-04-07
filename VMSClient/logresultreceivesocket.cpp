#include "logresultreceivesocket.h"
#include "socketbase.h"
#include "serverinfosocket.h"

#include <QtWidgets>

LogResultReceiveSocket::LogResultReceiveSocket(QObject *parent) :
    QThread(parent)
{
    m_chanelIndex = -1;
    m_serverInfoSocket = NULL;
}

void LogResultReceiveSocket::setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex)
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

ServerInfoSocket* LogResultReceiveSocket::serverInfoSocket()
{
    return m_serverInfoSocket;
}

int LogResultReceiveSocket::chanelIndex()
{
    return m_chanelIndex;
}

void LogResultReceiveSocket::stopSocket()
{
    closesocket(m_socket);

    wait();
}

void LogResultReceiveSocket::slotReconnect()
{
    stopSocket();

    start();
}

void LogResultReceiveSocket::slotChanelStatusChanged(int , int chanelIndex, int chanelStatus)
{
    if(chanelIndex != m_chanelIndex)
        return;

    if(chanelStatus == CHANEL_STATUS_STOP)
        stopSocket();
    else
        slotReconnect();
}

void LogResultReceiveSocket::run()
{
    SOCKADDR_IN target; //Information about host

    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
        return;

    if(m_chanelIndex < 0 || m_chanelIndex >= m_serverInfoSocket->serverInfo().ipCameraInfos.size())
        return;

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfoSocket->serverInfo().port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfoSocket->serverInfo().ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
        return;

    int ret = authenticateSocket();
    if(ret)
        recvFaceImage();

    closesocket(m_socket);
}


int LogResultReceiveSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfoSocket->serverInfo().userName + "\n" + m_serverInfoSocket->serverInfo().password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "FACE_IMAGE");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

void LogResultReceiveSocket::recvFaceImage()
{
    while(1)
    {
        QByteArray receiveData;
        int ret = receiveDataBySocket(m_socket, receiveData);
        if(!ret)
            break;

        QBuffer receiveDataBuffer(&receiveData);
        receiveDataBuffer.open(QIODevice::ReadOnly);

        int logCount = 0;
        QDataStream receiveDataStream(&receiveDataBuffer);

        receiveDataStream >> logCount;
        for(int i = 0; i < logCount; i ++)
        {
            QDateTime dateTime;
            QByteArray imageData;
            QString logUIDStr;

            receiveDataStream >> dateTime;
            receiveDataStream >> imageData;
            receiveDataStream >> logUIDStr;

            LOG_RESULT logResult;
            for(int j = 0; j < 3; j ++)
            {
                QByteArray candiateFace;
                receiveDataStream >> candiateFace;
                receiveDataStream >> logResult.candidateDist[j];
                receiveDataStream >> logResult.candidateType[j];

                logResult.candidateFace[j] = candiateFace;
            }

            logResult.serverUID = m_serverInfoSocket->serverInfo().serverUID;
            logResult.chanelIndex = m_chanelIndex;
            logResult.logUID = logUIDStr;
            logResult.dateTime = dateTime;
            logResult.faceImage = imageData;

            emit receivedLogResult(logResult);
        }
    }
}
