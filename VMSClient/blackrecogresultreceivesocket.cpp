#include "blackrecogresultreceivesocket.h"
#include "socketbase.h"
#include "serverinfosocket.h"

#include <QtWidgets>

BlackRecogResultReceiveSocket::BlackRecogResultReceiveSocket(QObject *parent) :
    QThread(parent)
{
    m_chanelIndex = -1;
    m_serverInfoSocket = NULL;
}

void BlackRecogResultReceiveSocket::setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex)
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

void BlackRecogResultReceiveSocket::stopSocket()
{
    closesocket(m_socket);

    wait();
}

ServerInfoSocket* BlackRecogResultReceiveSocket::serverInfoSocket()
{
    return m_serverInfoSocket;
}

int BlackRecogResultReceiveSocket::chanelIndex()
{
    return m_chanelIndex;
}

void BlackRecogResultReceiveSocket::slotReconnect()
{
    stopSocket();

    start();
}

void BlackRecogResultReceiveSocket::slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus)
{
    if(chanelIndex != m_chanelIndex)
        return;

    if(chanelStatus == CHANEL_STATUS_STOP)
        stopSocket();
    else
        slotReconnect();
}

void BlackRecogResultReceiveSocket::run()
{
    WSADATA w; //Winsock startup info
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
        receiveBlackRecogResult();

    closesocket(m_socket);
}

int BlackRecogResultReceiveSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfoSocket->serverInfo().userName + "\n" + m_serverInfoSocket->serverInfo().password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "BLACK_RECOG_RESULT");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

void BlackRecogResultReceiveSocket::receiveBlackRecogResult()
{
    while(1)
    {
        QByteArray receivedFaceData;
        int ret = receiveDataBySocket(m_socket, receivedFaceData);
        if(!ret)
            break;

        if(receivedFaceData.size())
        {
            QBuffer receiveDataBuffer(&receivedFaceData);
            receiveDataBuffer.open(QIODevice::ReadOnly);

            QDataStream receiveDataStream(&receiveDataBuffer);
            int blackRecogResultCount = 0;
            receiveDataStream >> blackRecogResultCount;

            for(int i = 0; i < blackRecogResultCount; i ++)
            {
                int candiateCount = 0;
                receiveDataStream >> candiateCount;

                for(int j = 0; j < candiateCount; j ++)
                {
                    QVector<BLACK_RECOG_RESULT> cadidateInfos;

                    BLACK_RECOG_RESULT blackRecogResult;
                    receiveDataStream >> blackRecogResult.name;
                    receiveDataStream >> blackRecogResult.gender;
                    receiveDataStream >> blackRecogResult.birthday;
                    receiveDataStream >> blackRecogResult.address;
                    receiveDataStream >> blackRecogResult.description;
                    receiveDataStream >> blackRecogResult.personType;
                    receiveDataStream >> blackRecogResult.galleryFaceData;
                    receiveDataStream >> blackRecogResult.probeFaceData;
                    receiveDataStream >> blackRecogResult.similiarity;

                    receiveDataStream >> blackRecogResult.logUID;
                    receiveDataStream >> blackRecogResult.dateTime;

                    blackRecogResult.serverUID = m_serverInfoSocket->serverInfo().serverUID;
                    blackRecogResult.chanelIndex = m_chanelIndex;

                    cadidateInfos.append(blackRecogResult);

                    emit receivedBlackRecogResult(cadidateInfos);
                }
            }

            receiveDataBuffer.close();

        }
    }
}
