#include "searchlogsocket.h"
#include "socketbase.h"
#include "serverinfosocket.h"
#include "clientbase.h"

#include <QtWidgets>

SearchLogSocket::SearchLogSocket(QObject *parent) :
    QThread(parent)
{
    m_running = 0;
}

void SearchLogSocket::startSocket(QVector<int> serverIndexs, QVector<int> chanelIndexs, QVector<QString> areaNames, QVector<ServerInfoSocket*> serverInfoSockets,
                    QDateTime startTime, QDateTime endTime, QByteArray featData, int threshold)
{
    m_serverIndexs = serverIndexs;
    m_chanelIndexs = chanelIndexs;
    m_areaNames = areaNames;
    m_serverInfoSockets = serverInfoSockets;

    m_startTime = startTime;
    m_endTime = endTime;
    m_featData = featData;
    m_threshold = threshold;

    start();
}

void SearchLogSocket::stopSocket()
{
    closesocket(m_socket);

    m_mutex.lock();
    m_running = 0;
    m_mutex.unlock();

    wait();
}

void SearchLogSocket::run()
{
    FILE* fp = fopen(CACHE_LOG_NAME, "wb");
    fclose(fp);

    m_running = 1;
    m_receivedLogCount = 0;
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
            receiveLogs();

        closesocket(m_socket);
    }

    emit socketFinished(this);
}

int SearchLogSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "SEARCH LOG");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

void SearchLogSocket::receiveLogs()
{
    QByteArray sendData;
    QBuffer sendBuffer(&sendData);
    sendBuffer.open(QIODevice::WriteOnly);

    QDataStream sendDataStream(&sendBuffer);

    sendDataStream << m_startTime;
    sendDataStream << m_endTime;
    sendDataStream << m_featData;
    sendDataStream << m_threshold;

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
        QDateTime dateTime;
        QByteArray imageData;
        QString logUIDStr;

        receiveDataStream >> dateTime;
        receiveDataStream >> imageData;
        receiveDataStream >> logUIDStr;

        LOG_RESULT logResult;
        logResult.serverUID = m_serverInfo.serverUID;
        logResult.chanelIndex = m_chanelIndex;
        logResult.logUID = logUIDStr;
        logResult.dateTime = dateTime;
        logResult.faceImage = imageData;
        logResult.areaName = m_areaName;

        QByteArray resultData;
        QBuffer resultBuffer(&resultData);
        resultBuffer.open(QIODevice::WriteOnly);
        QDataStream resultStream(&resultBuffer);

        resultStream << logResult.serverUID;
        resultStream << logResult.chanelIndex;
        resultStream << logResult.logUID;
        resultStream << logResult.dateTime;
        resultStream << logResult.faceImage;
        resultStream << logResult.areaName;

        resultBuffer.close();

        resultData.resize(CACHE_LOG_SIZE);

        FILE* fp = fopen(CACHE_LOG_NAME, "r+b");
        if(fp)
        {
            fseek(fp, CACHE_LOG_SIZE * m_receivedLogCount, SEEK_SET);
            fwrite(resultData.data(), resultData.size(), 1, fp);
            fclose(fp);

            m_receivedLogCount ++;
            emit receivedLogResult(logResult);
        }
    }
}
