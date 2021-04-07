#include "searchblacksocket.h"
#include "socketbase.h"
#include "serverinfosocket.h"
#include "clientbase.h"

#include <QtWidgets>

SearchBlackSocket::SearchBlackSocket(QObject *parent) :
    QThread(parent)
{
}

void SearchBlackSocket::startSocket(QVector<int> serverIndexs, QVector<int> chanelIndexs, QVector<QString> areaNames, QVector<ServerInfoSocket*> serverInfoSockets,
                    QDateTime startTime, QDateTime endTime, QString searchName)
{
    m_serverIndexs = serverIndexs;
    m_chanelIndexs = chanelIndexs;
    m_areaNames = areaNames;
    m_serverInfoSockets = serverInfoSockets;

    m_startTime = startTime;
    m_endTime = endTime;
    m_searchName = searchName;

    start();
}


void SearchBlackSocket::stopSocket()
{
    m_mutex.lock();
    m_running = 0;
    m_mutex.unlock();
    closesocket(m_socket);
    wait();
}

void SearchBlackSocket::run()
{
    FILE* fp = fopen(CACHE_BLACK_NAME, "wb");
    fclose(fp);

    m_running = 1;
    m_receivedBlackCount = 0;
    for(int i = 0; i < m_serverIndexs.size(); i ++)
    {
        m_mutex.lock();
        int running = m_running;
        m_mutex.unlock();
        if(running == 0)
            break;

        SOCKADDR_IN target;
        m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
        if (m_socket == INVALID_SOCKET)
            continue;

        m_serverInfo = m_serverInfoSockets[m_serverIndexs[i]]->serverInfo();
        m_chanelIndex = m_chanelIndexs[i];
        m_areaName = m_areaNames[i];

        target.sin_family = AF_INET;           // address family Internet
        target.sin_port = htons (m_serverInfo.port);        // set server’s port number
        target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

        if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
            continue;

        int ret = authenticateSocket();
        if(ret)
            receiveBlackResults();

        closesocket(m_socket);
    }

    emit socketFinished(this);
}

int SearchBlackSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "SEARCH BLACK");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

void SearchBlackSocket::receiveBlackResults()
{
    QByteArray sendData;
    QBuffer sendBuffer(&sendData);
    sendBuffer.open(QIODevice::WriteOnly);

    QDataStream sendDataStream(&sendBuffer);

    sendDataStream << m_startTime;
    sendDataStream << m_endTime;
    sendDataStream << m_searchName;

    sendBuffer.close();

    int ret = sendDataBySocket(m_socket, sendData);;
    if(!ret)
        return;

    while(1)
    {
        QByteArray receiveData;
        ret = receiveDataBySocket(m_socket, receiveData);

        if(!ret)
            break;

        QBuffer receiveDataBuffer(&receiveData);
        receiveDataBuffer.open(QIODevice::ReadOnly);

        QDataStream receiveDataStream(&receiveDataBuffer);
        int candiateCount = 0;
        receiveDataStream >> candiateCount;

        QVector<BLACK_RECOG_RESULT> cadidateInfos;

        for(int j = 0; j < candiateCount; j ++)
        {
            BLACK_RECOG_RESULT blackRecogResult;
            int blackRecogResultSize;
            receiveDataStream >> blackRecogResultSize;
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

            blackRecogResult.serverUID = m_serverInfo.serverUID;
            blackRecogResult.chanelIndex = m_chanelIndex;
            blackRecogResult.areaName = m_areaName;

            cadidateInfos.append(blackRecogResult);
        }

        QByteArray resultData;
        QBuffer resultBuffer(&resultData);
        resultBuffer.open(QIODevice::WriteOnly);
        QDataStream resultStream(&resultBuffer);

        resultStream << cadidateInfos[0].name;
        resultStream << cadidateInfos[0].gender;
        resultStream << cadidateInfos[0].birthday;
        resultStream << cadidateInfos[0].address;
        resultStream << cadidateInfos[0].description;
        resultStream << cadidateInfos[0].personType;
        resultStream << cadidateInfos[0].galleryFaceData;
        resultStream << cadidateInfos[0].probeFaceData;
        resultStream << cadidateInfos[0].similiarity;
        resultStream << cadidateInfos[0].areaName;
        resultStream << cadidateInfos[0].logUID;
        resultStream << cadidateInfos[0].dateTime;
        resultStream << cadidateInfos[0].serverUID;
        resultStream << cadidateInfos[0].chanelIndex;

        resultBuffer.close();

        resultData.resize(CACHE_BLACK_SIZE);

        if(cadidateInfos.size())
        {
            FILE* fp = fopen(CACHE_BLACK_NAME, "r+b");
            if(fp)
            {
                fseek(fp, CACHE_BLACK_SIZE * m_receivedBlackCount, SEEK_SET);
                fwrite(resultData.data(), resultData.size(), 1, fp);
                fclose(fp);
                m_receivedBlackCount ++;

                emit receivedBlackRecogResult(cadidateInfos);
            }
        }
    }
}
