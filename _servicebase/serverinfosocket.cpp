#include "serverinfosocket.h"
#include "socketbase.h"
#include "servicebase.h"
#include "cameraprocessengine.h"

ServerInfoSocket::ServerInfoSocket(QObject *parent) :
    QThread(parent)
{
    m_socket = INVALID_SOCKET;
    m_blackInfoChanged = 0;
}

void ServerInfoSocket::startSocket(SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;

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

void ServerInfoSocket::slotBlackInfoChanged()
{
    m_blackInfoMutex.lock();
    m_blackInfoChanged = 1;
    m_blackInfoMutex.unlock();
}

void ServerInfoSocket::setCameraProcessEngines(QVector<CameraProcessEngine*> cameraProcessEngines)
{
    m_cameraProcessEngines = cameraProcessEngines;
}

void ServerInfoSocket::run()
{
    sendStringBySocket(m_socket, getServerInfoStr());
    sendIntBySocket(m_socket, getHDSerial());
    sendIpCameraInfos();

    qint64 curTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    send(m_socket, (char*)&curTime, sizeof(qint64), 0);

    QByteArray cameraStatus;
    cameraStatus.resize(m_cameraProcessEngines.size());
    memset(cameraStatus.data(), 0, cameraStatus.size());

    while(1)
    {
        QString receiveStr;
        int ret = receiveStringBySocket(m_socket, receiveStr);
        if(!ret)
            break;

        if(receiveStr == "NONE")
        {
            sendStringBySocket(m_socket, receiveStr);

            QByteArray newCameraStatus;
            newCameraStatus.resize(m_cameraProcessEngines.size());

            int changed = 0;
            for(int i = 0; i < m_cameraProcessEngines.size(); i ++)
            {
                newCameraStatus[i] = m_cameraProcessEngines[i]->status();
                if(newCameraStatus[i] != cameraStatus[i])
                    changed = 1;
            }

            m_blackInfoMutex.lock();
            int blackInfoChanged = m_blackInfoChanged;
            m_blackInfoMutex.unlock();

            if(changed || blackInfoChanged)
            {
                cameraStatus = newCameraStatus;
                sendStringBySocket(m_socket, "CAMARE_STATUS");
                sendDataBySocket(m_socket, cameraStatus);

                for(int i = 0; i < m_cameraProcessEngines.size(); i ++)
                {
                    int blackInfoSize = getIpCameraInfo(i).blackInfoSize;
                    sendIntBySocket(m_socket, blackInfoSize);
                }

                m_blackInfoChanged = 0;
            }
        }
        else if(receiveStr == "IP CAMERA")
        {
            QVector<IpCameraInfo> ipCameraInfos;

            QByteArray receiveData;
            ret = receiveDataBySocket(m_socket, receiveData);
            if(!ret)
                break;

            QBuffer receiveDataBuffer(&receiveData);
            receiveDataBuffer.open(QIODevice::ReadOnly);

            int chanelCount = 0;
            QDataStream receiveDataStream(&receiveDataBuffer);

            receiveDataStream >> chanelCount;
            for(int i = 0; i < chanelCount; i ++)
            {
                IpCameraInfo ipCameraInfo;
                receiveDataStream >> ipCameraInfo.ipAddress;
                receiveDataStream >> ipCameraInfo.portNum;
                receiveDataStream >> ipCameraInfo.videoSource;
                receiveDataStream >> ipCameraInfo.streamUri;

                receiveDataStream >> ipCameraInfo.chkFaceSize;
                receiveDataStream >> ipCameraInfo.detectionFaceMinSize;
                receiveDataStream >> ipCameraInfo.chkThreshold;
                receiveDataStream >> ipCameraInfo.identifyThreshold;
                receiveDataStream >> ipCameraInfo.chkBlackCanidiateCount;
                receiveDataStream >> ipCameraInfo.blackCandidateCount;

                ipCameraInfos.append(ipCameraInfo);
            }

            receiveDataBuffer.close();

            emit receiveIpCameraInfos(ipCameraInfos);
        }
        else if(receiveStr == "SURVAILLANCE SETTING")
        {
            QByteArray receiveData;
            ret = receiveDataBySocket(m_socket, receiveData);
            if(!ret)
                break;

            QBuffer receiveDataBuffer(&receiveData);
            receiveDataBuffer.open(QIODevice::ReadOnly);

            QDataStream receiveDataStream(&receiveDataBuffer);

            int chanelIndex = 0;
            IpCameraInfo ipCameraInfo;
            receiveDataStream >> ipCameraInfo.chkFaceSize;
            receiveDataStream >> ipCameraInfo.detectionFaceMinSize;
            receiveDataStream >> ipCameraInfo.chkThreshold;
            receiveDataStream >> ipCameraInfo.identifyThreshold;
            receiveDataStream >> ipCameraInfo.chkBlackCanidiateCount;
            receiveDataStream >> ipCameraInfo.blackCandidateCount;
            receiveDataStream >> chanelIndex;

            receiveDataBuffer.close();

            emit receiveSurveillanceSetting(ipCameraInfo, chanelIndex);
        }
        else if(receiveStr == "ONE CAMERA SETTING")
        {
            QByteArray receiveData;
            ret = receiveDataBySocket(m_socket, receiveData);
            if(!ret)
                break;

            QBuffer receiveDataBuffer(&receiveData);
            receiveDataBuffer.open(QIODevice::ReadOnly);

            QDataStream receiveDataStream(&receiveDataBuffer);

            int chanelIndex = 0;
            IpCameraInfo ipCameraInfo;
            receiveDataStream >> ipCameraInfo.ipAddress;
            receiveDataStream >> ipCameraInfo.portNum;
            receiveDataStream >> ipCameraInfo.videoSource;
            receiveDataStream >> ipCameraInfo.streamUri;
            receiveDataStream >> chanelIndex;

            receiveDataBuffer.close();

            emit receiveIpCameraInfo(ipCameraInfo, chanelIndex);
        }
        else
            break;
    }

    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
}

void ServerInfoSocket::sendIpCameraInfos()
{
    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);
    sendingDataStream << getChanelCount();

    QVector<IpCameraInfo> ipCameraInfos = getIpCameraInfos();
    for(int i = 0; i < ipCameraInfos.size(); i ++)
    {
        sendingDataStream << ipCameraInfos[i].ipAddress;
        sendingDataStream << ipCameraInfos[i].portNum;
        sendingDataStream << ipCameraInfos[i].videoSource;
        sendingDataStream << ipCameraInfos[i].streamUri;

        sendingDataStream << ipCameraInfos[i].chkFaceSize;
        sendingDataStream << ipCameraInfos[i].detectionFaceMinSize;
        sendingDataStream << ipCameraInfos[i].chkThreshold;
        sendingDataStream << ipCameraInfos[i].identifyThreshold;
        sendingDataStream << ipCameraInfos[i].chkBlackCanidiateCount;
        sendingDataStream << ipCameraInfos[i].blackCandidateCount;

        sendingDataStream << ipCameraInfos[i].blackInfoSize;
    }

    sendingBuffer.close();

    sendDataBySocket(m_socket, sendingData);
}

