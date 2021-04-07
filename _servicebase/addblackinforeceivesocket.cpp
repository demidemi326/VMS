#include "addblackinforeceivesocket.h"
#include "socketbase.h"

#include <QtWidgets>

AddBlackInfoReceiveSocket::AddBlackInfoReceiveSocket(QObject *parent) :
    QThread(parent)
{
}

void AddBlackInfoReceiveSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;

    start();
}

void AddBlackInfoReceiveSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void AddBlackInfoReceiveSocket::run()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
    {
        emit receiveFinished(this);
        return;
    }

//    QBuffer receiveDataBuffer(&receiveData);
//    receiveDataBuffer.open(QIODevice::ReadOnly);

//    QDataStream receiveDataStream(&receiveDataBuffer);

//    int blackInfoCount = 0;
//    receiveDataStream >> blackInfoCount;

//    for(int i = 0; i < blackInfoCount; i ++)
//    {
//        QVector<QByteArray> featData;
//        receiveDataStream >> featData;

//        Person1 personFeat;
//        personFeat.nFeatureNum = featData.size();

//        personFeat.pxFeatures = (ARM_Feature*)malloc(sizeof(ARM_Feature) * personFeat.nFeatureNum);
//        for(int j = 0; j < personFeat.nFeatureNum; j ++)
//            memcpy(&personFeat.pxFeatures[j], featData[j].data(), featData[j].size());

//        BlackPersonMetaInfo personMetaInfo;
//        receiveDataStream >> personMetaInfo.name;
//        receiveDataStream >> personMetaInfo.gender;
//        receiveDataStream >> personMetaInfo.birthday;
//        receiveDataStream >> personMetaInfo.address;
//        receiveDataStream >> personMetaInfo.description;
//        receiveDataStream >> personMetaInfo.personType;
//        receiveDataStream >> personMetaInfo.faceData;

//        emit receiveBlackPersoInfo(personFeat, personMetaInfo);
//    }

//    receiveDataBuffer.close();
    emit receivedBlackPersonData(receiveData);
    emit receiveFinished(this);
}
