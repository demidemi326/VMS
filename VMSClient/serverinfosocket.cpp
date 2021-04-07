#include "serverinfosocket.h"
#include "socketbase.h"

#include <QtWidgets>

ServerInfoSocket::ServerInfoSocket(QObject *parent) :
    QThread(parent)
{
    m_diffTime = 0;
    m_socket = INVALID_SOCKET;
    setStatus(SERVER_STOP);
    connect(this, SIGNAL(finished()), this, SLOT(start()));
}

void ServerInfoSocket::startSocket(ServerInfo serverInfo, int serverIndex)
{
    m_serverInfo = serverInfo;
    m_serverIndex = serverIndex;

    setStatus(SERVER_STOP);

    start();
}

void ServerInfoSocket::stopSocket()
{
    if(m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    wait();
}

void ServerInfoSocket::changeServerInfo(ServerInfo serverInfo)
{
    m_serverInfo = serverInfo;
    stopSocket();
}

void ServerInfoSocket::setIpCameraInfos(QVector<IpCameraInfo> ipCameraInfos)
{
    m_cameraMutex.lock();
    m_serverInfo.ipCameraInfos = ipCameraInfos;
    m_cameraMutex.unlock();

    setStatus(SERVER_SETTING_IPCAMERA);
}

void ServerInfoSocket::setSurveillanceSetting(IpCameraInfo ipCameraInfo, int chanelIndex)
{
    m_cameraMutex.lock();
    m_tmpCameraInfo = ipCameraInfo;
    m_tmpChanelIndex = chanelIndex;
    m_cameraMutex.unlock();

    setStatus(SERVER_SETTING_SURVEILLANCE);
}

void ServerInfoSocket::setIpCameraInfo(IpCameraInfo ipCameraInfo, int chanelIndex)
{
    m_cameraMutex.lock();
    m_tmpCameraInfo = ipCameraInfo;
    m_tmpChanelIndex = chanelIndex;
    m_cameraMutex.unlock();

    setStatus(SERVER_SETTING_ONE_IPCAMERA);
}

ServerInfo ServerInfoSocket::serverInfo()
{
    return m_serverInfo;
}

int ServerInfoSocket::status()
{
    m_mutex.lock();
    int status = m_status;
    m_mutex.unlock();

    return status;
}

void ServerInfoSocket::setStatus(int status, int sendStatus)
{
    m_mutex.lock();
    if(m_status == status)
    {
        m_mutex.unlock();
        return;
    }
    m_status = status;
    m_mutex.unlock();

    if(sendStatus)
        emit statusChanged(m_serverIndex, status);
}

int ServerInfoSocket::getChanelStatus(int chanelIndex)
{
    m_cameraStatusMutex.lock();
    if(chanelIndex < 0 || chanelIndex >= m_cameraStatus.size())
    {
        m_cameraStatusMutex.unlock();
        return CHANEL_STATUS_STOP;
    }

    int status = m_cameraStatus[chanelIndex];
    m_cameraStatusMutex.unlock();

    return status;
}

qint64 ServerInfoSocket::diffTime()
{
    return m_diffTime;
}

void ServerInfoSocket::run()
{
    setStatus(SERVER_STOP);
    SOCKADDR_IN target; //Information about host

    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
    {
        QThread::msleep(500);
        return;
    }

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfo.port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
    {
        QThread::msleep(500);
        return;
    }

    int ret = authenticateSocket();
    if(ret)
        receiveServerInfo();

    if (m_socket)
        closesocket(m_socket);

    setStatus(SERVER_STOP, 1);
}

int ServerInfoSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "SERVER_INFO");
        return 1;
    }
    else
        return 0;
}

void ServerInfoSocket::receiveServerInfo()
{
    QString serverInfoStr;
    int ret = receiveStringBySocket(m_socket, serverInfoStr);
    if(!ret)
        return;

    int serverUID;
    receiveIntBySocket(m_socket, serverUID);

    qint64 oldTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    QByteArray receiveData;
    ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
        return;

    qint64 serverTime;
    recv(m_socket, (char*)&serverTime, sizeof(qint64), 0);
    qint64 curTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_diffTime = serverTime - curTime;

    setStatus(SERVER_START);

    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    int chanelCount = 0;
    QDataStream receiveDataStream(&receiveDataBuffer);

    m_serverInfo.ipCameraInfos.resize(0);
    receiveDataStream >> chanelCount;
    for(int i = 0; i < chanelCount; i ++)
    {
        IpCameraInfo ipCameraInfo;
        receiveDataStream >> ipCameraInfo.ipAddress;
        receiveDataStream >> ipCameraInfo.portNum;
        receiveDataStream >> ipCameraInfo.videoSource;
        receiveDataStream >> ipCameraInfo.streamuri;

        receiveDataStream >> ipCameraInfo.chkFaceSize;
        receiveDataStream >> ipCameraInfo.detectionFaceMinSize;
        receiveDataStream >> ipCameraInfo.chkThreshold;
        receiveDataStream >> ipCameraInfo.identifyThreshold;
        receiveDataStream >> ipCameraInfo.chkBlackCandidateCount;
        receiveDataStream >> ipCameraInfo.blackCandidateCount;

        receiveDataStream >> ipCameraInfo.blackInfoSize;

        m_serverInfo.ipCameraInfos.append(ipCameraInfo);
    }

    receiveDataBuffer.close();

    m_cameraStatusMutex.lock();
    m_cameraStatus.resize(chanelCount);
    memset(m_cameraStatus.data(), CHANEL_STATUS_STOP, m_cameraStatus.size());
    m_cameraStatusMutex.unlock();

    m_serverInfo.serverTypeStr = serverInfoStr;
    m_serverInfo.serverUID = serverUID;

    setStatus(SERVER_NONE, 1);
    while(1)
    {
        if(status() == SERVER_STOP)
            break;

        if(status() == SERVER_NONE)
        {
            QThread::msleep(500);

            sendStringBySocket(m_socket, "NONE");

            QString noneStr;
            receiveStringBySocket(m_socket, noneStr);
            if(noneStr == "CAMARE_STATUS")
            {
                QByteArray cameraStatus;
                receiveDataBySocket(m_socket, cameraStatus);

                for(int i = 0; i < m_serverInfo.ipCameraInfos.size(); i ++)
                {
                    int blackInfoSize;
                    receiveIntBySocket(m_socket, blackInfoSize);

                    if(m_serverInfo.ipCameraInfos[i].blackInfoSize != blackInfoSize)
                    {
                        m_serverInfo.ipCameraInfos[i].blackInfoSize = blackInfoSize;
                        emit blackInfoChanged(m_serverIndex, i);
                    }
                }

                if(cameraStatus.size() != m_serverInfo.ipCameraInfos.size())
                    break;

                m_cameraStatusMutex.lock();
                for(int i = 0; i < m_cameraStatus.size(); i ++)
                {
                    if(m_cameraStatus[i] != cameraStatus[i])
                    {
                        emit chanelStatusChanged(m_serverIndex, i, (int)cameraStatus[i]);
                    }
                }

                m_cameraStatus = cameraStatus;
                m_cameraStatusMutex.unlock();
            }
            else if(noneStr != "NONE")
                break;
        }
        else if(status() == SERVER_SETTING_IPCAMERA)
        {
            QThread::msleep(100);

            sendStringBySocket(m_socket, "IP CAMERA");

            QByteArray sendingData;
            QBuffer sendingBuffer(&sendingData);
            sendingBuffer.open(QIODevice::WriteOnly);

            QDataStream sendingDataStream(&sendingBuffer);
            sendingDataStream << m_serverInfo.ipCameraInfos.size();

            for(int i = 0; i < m_serverInfo.ipCameraInfos.size(); i ++)
            {
                sendingDataStream << m_serverInfo.ipCameraInfos[i].ipAddress;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].portNum;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].videoSource;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].streamuri;

                sendingDataStream << m_serverInfo.ipCameraInfos[i].chkFaceSize;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].detectionFaceMinSize;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].chkThreshold;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].identifyThreshold;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].chkBlackCandidateCount;
                sendingDataStream << m_serverInfo.ipCameraInfos[i].blackCandidateCount;
            }

            sendingBuffer.close();

            sendDataBySocket(m_socket, sendingData);

            setStatus(SERVER_NONE);
        }
        else if(status() == SERVER_SETTING_SURVEILLANCE)
        {
            sendStringBySocket(m_socket, "SURVAILLANCE SETTING");

            QByteArray sendingData;
            QBuffer sendingBuffer(&sendingData);
            sendingBuffer.open(QIODevice::WriteOnly);

            QDataStream sendingDataStream(&sendingBuffer);
            m_cameraMutex.lock();
            IpCameraInfo tmpCameraInfo = m_tmpCameraInfo;
            int tmpChanelIndex = m_tmpChanelIndex;
            m_cameraMutex.unlock();

            sendingDataStream << tmpCameraInfo.chkFaceSize;
            sendingDataStream << tmpCameraInfo.detectionFaceMinSize;
            sendingDataStream << tmpCameraInfo.chkThreshold;
            sendingDataStream << tmpCameraInfo.identifyThreshold;
            sendingDataStream << tmpCameraInfo.chkBlackCandidateCount;
            sendingDataStream << tmpCameraInfo.blackCandidateCount;
            sendingDataStream << tmpChanelIndex;

            sendingBuffer.close();

            int ret = sendDataBySocket(m_socket, sendingData);
            if(ret)
            {
                if(tmpChanelIndex >= 0 && tmpChanelIndex < m_serverInfo.ipCameraInfos.size())
                {
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].chkFaceSize = tmpCameraInfo.chkFaceSize;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].detectionFaceMinSize = tmpCameraInfo.detectionFaceMinSize;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].chkThreshold = tmpCameraInfo.chkThreshold;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].identifyThreshold = tmpCameraInfo.identifyThreshold;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].chkBlackCandidateCount = tmpCameraInfo.chkBlackCandidateCount;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].blackCandidateCount = tmpCameraInfo.blackCandidateCount;
                }
            }

            setStatus(SERVER_NONE);
            QThread::msleep(10);
        }
        else if(status() == SERVER_SETTING_ONE_IPCAMERA)
        {
            sendStringBySocket(m_socket, "ONE CAMERA SETTING");

            QByteArray sendingData;
            QBuffer sendingBuffer(&sendingData);
            sendingBuffer.open(QIODevice::WriteOnly);

            QDataStream sendingDataStream(&sendingBuffer);

            m_cameraMutex.lock();
            IpCameraInfo tmpCameraInfo = m_tmpCameraInfo;
            int tmpChanelIndex = m_tmpChanelIndex;
            m_cameraMutex.unlock();

            sendingDataStream << tmpCameraInfo.ipAddress;
            sendingDataStream << tmpCameraInfo.portNum;
            sendingDataStream << tmpCameraInfo.videoSource;
            sendingDataStream << tmpCameraInfo.streamuri;
            sendingDataStream << m_tmpChanelIndex;

            sendingBuffer.close();

            int ret = sendDataBySocket(m_socket, sendingData);
            if(ret)
            {
                if(tmpChanelIndex >= 0 && tmpChanelIndex < m_serverInfo.ipCameraInfos.size())
                {
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].ipAddress = tmpCameraInfo.ipAddress;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].portNum = tmpCameraInfo.portNum;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].videoSource = tmpCameraInfo.videoSource;
                    m_serverInfo.ipCameraInfos[tmpChanelIndex].streamuri = tmpCameraInfo.streamuri;
                }
            }

            setStatus(SERVER_NONE);
            QThread::msleep(10);
        }
    }

    m_cameraStatusMutex.lock();
    for(int i = 0; i < m_cameraStatus.size(); i ++)
    {
        m_cameraStatus[i] = CHANEL_STATUS_STOP;
        emit chanelStatusChanged(m_serverIndex, i, CHANEL_STATUS_STOP);
    }
    m_cameraStatusMutex.unlock();
    m_diffTime = 0;
}

