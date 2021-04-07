#include "searchlastblacksocket.h"
#include "serverinfosocket.h"

#include "socketbase.h"
#include <QtWidgets>

SearchLastBlackSocket::SearchLastBlackSocket(QObject *parent) :
    QThread(parent)
{
}

void SearchLastBlackSocket::startSocket(QVector<int> serverIndexs, QVector<int> chanelIndexs, QVector<ServerInfoSocket*> serverInfoSockets)
{
    m_serverIndexs = serverIndexs;
    m_chanelIndexs = chanelIndexs;
    m_serverInfoSockets = serverInfoSockets;

    start();
}

void SearchLastBlackSocket::stopSocket()
{
    m_mutex.lock();
    m_running = 0;
    m_mutex.unlock();
    closesocket(m_socket);
    wait();
}

bool lessThan( const BLACK_RECOG_RESULT & e1, const BLACK_RECOG_RESULT & e2 )
{
    if(e1.logUID > e2.logUID)
        return false;
    else
        return true;
}

void SearchLastBlackSocket::run()
{
#define N_MAX_BLACK_RESULT 100
//    QThread::msleep(1000);

    m_running = 1;
    m_receivedBlackCount = 0;
    for(int i = 0; i < m_serverIndexs.size(); i ++)
    {
        m_mutex.lock();
        int running = m_running;
        m_mutex.unlock();
        if(running == 0)
            break;

        m_serverInfo = m_serverInfoSockets[m_serverIndexs[i]]->serverInfo();
        m_chanelIndex = m_chanelIndexs[i];
        if(m_serverInfoSockets[m_serverIndexs[i]]->status() == SERVER_STOP)
            continue;

        SOCKADDR_IN target;
        m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
        if (m_socket == INVALID_SOCKET)
            continue;

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

    qSort(m_blackRecogResult.begin(), m_blackRecogResult.end(), lessThan);
    if(m_blackRecogResult.size() > N_MAX_BLACK_RESULT)
        m_blackRecogResult.remove(0, m_blackRecogResult.size() - N_MAX_BLACK_RESULT);

    for(int i = 0; i < m_blackRecogResult.size(); i ++)
    {
        QVector<BLACK_RECOG_RESULT> blackRecogResult;
        blackRecogResult << m_blackRecogResult[i];

        emit receivedLastBlackResults(blackRecogResult);
    }

    emit socketFinished(this);
}

int SearchLastBlackSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "SEARCH LAST BLACK");
        sendIntBySocket(m_socket, m_chanelIndex);
        return 1;
    }
    else
        return 0;
}

void SearchLastBlackSocket::receiveBlackResults()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);

    if(!ret)
        return;

    QVector<QByteArray> receiveBlackData;
    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    QDataStream receiveDataStream(&receiveDataBuffer);

    receiveDataStream >> receiveBlackData;
    receiveDataBuffer.close();

    for(int i = 0; i < receiveBlackData.size(); i ++)
    {
        BLACK_RECOG_RESULT blackRecogResult;
        QByteArray blackData = receiveBlackData[i];

        QBuffer blackDataBuffer(&blackData);
        blackDataBuffer.open(QIODevice::ReadOnly);

        QDataStream blackDataStream(&blackDataBuffer);

        blackDataStream >> blackRecogResult.name;

        blackDataStream >> blackRecogResult.gender;
        blackDataStream >> blackRecogResult.birthday;
        blackDataStream >> blackRecogResult.address;
        blackDataStream >> blackRecogResult.description;
        blackDataStream >> blackRecogResult.personType;
        blackDataStream >> blackRecogResult.galleryFaceData;
        blackDataStream >> blackRecogResult.probeFaceData;
        blackDataStream >> blackRecogResult.similiarity;

        blackDataStream >> blackRecogResult.logUID;
        blackDataStream >> blackRecogResult.dateTime;

        blackRecogResult.serverUID = m_serverInfo.serverUID;
        blackRecogResult.chanelIndex = m_chanelIndex;

        blackDataBuffer.close();

        m_blackRecogResult.append(blackRecogResult);
    }
}
