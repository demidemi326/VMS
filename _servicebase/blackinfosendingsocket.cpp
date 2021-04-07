#include "blackinfosendingsocket.h"
#include "socketbase.h"

#include <QtWidgets>

BlackInfoSendingSocket::BlackInfoSendingSocket(QObject *parent) :
    QThread(parent)
{
}

void BlackInfoSendingSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;
}

void BlackInfoSendingSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void BlackInfoSendingSocket::sendBlackInfo(PersonDatabase1 personDatabase, QVector<BlackPersonMetaInfo> metaInfo)
{
    m_blackMetaInfo = metaInfo;
    m_personDatabase = personDatabase;
    start();
}

void BlackInfoSendingSocket::run()
{
    if(m_blackMetaInfo.size() != m_personDatabase.nPersonNum)
    {
        emit sendingFinished(this);
        return;
    }

    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << m_blackMetaInfo.size();

    for(int i = 0; i < m_blackMetaInfo.size(); i ++)
    {
        QVector<QByteArray> featData;
        for(int j = 0; j < m_personDatabase.pxPersons[i].nFeatureNum; j ++)
        {
            QByteArray feat((char*)&m_personDatabase.pxPersons[i].pxFeatures[j], sizeof(ARM_Feature));
            featData.append(feat);
        }

        sendingDataStream << featData;
        sendingDataStream << m_blackMetaInfo[i].name;
        sendingDataStream << m_blackMetaInfo[i].gender;
        sendingDataStream << m_blackMetaInfo[i].birthday;
        sendingDataStream << m_blackMetaInfo[i].address;
        sendingDataStream << m_blackMetaInfo[i].description;
        sendingDataStream << m_blackMetaInfo[i].personType;
        sendingDataStream << m_blackMetaInfo[i].faceData;
    }

    sendingBuffer.close();

    sendDataBySocket(m_socket, sendingData);

    emit sendingFinished(this);
}

