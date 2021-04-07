#include "oneblackpersoneditsocket.h"
#include "socketbase.h"

#include <QtWidgets>

OneBlackPersonEditSocket::OneBlackPersonEditSocket(QObject *parent) :
    QThread(parent)
{
}

void OneBlackPersonEditSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;

    start();
}

void OneBlackPersonEditSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void OneBlackPersonEditSocket::run()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
    {
        emit receiveEditInfoFinished(this);
        return;
    }

    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    QDataStream receiveDataStream(&receiveDataBuffer);

    QString oldName;
    receiveDataStream >> oldName;

    BlackPersonMetaInfo personMetaInfo;
    receiveDataStream >> personMetaInfo.name;
    receiveDataStream >> personMetaInfo.gender;
    receiveDataStream >> personMetaInfo.birthday;
    receiveDataStream >> personMetaInfo.address;
    receiveDataStream >> personMetaInfo.description;
    receiveDataStream >> personMetaInfo.personType;
    receiveDataStream >> personMetaInfo.faceData;

    QVector<QByteArray> featData;
    receiveDataStream >> featData;

    Person1 personFeat;
    personFeat.nFeatureNum = featData.size();
    personFeat.pxFeatures = (ARM_Feature*)malloc(sizeof(ARM_Feature) * featData.size());
    for(int i = 0; i < featData.size(); i ++)
        memcpy(&personFeat.pxFeatures[i], featData[i].data(), featData[i].size());

    receiveDataBuffer.close();

    emit receiveEditBlackPersonInfo(oldName, personFeat, personMetaInfo);
    emit receiveEditInfoFinished(this);
}
