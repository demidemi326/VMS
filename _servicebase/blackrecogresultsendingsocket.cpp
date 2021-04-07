#include "blackrecogresultsendingsocket.h"
#include "socketbase.h"

#include <QtWidgets>

BlackRecogResultSendingSocket::BlackRecogResultSendingSocket(QObject *parent) :
    QThread(parent)
{
    m_chanelIndex = -1;
}

void BlackRecogResultSendingSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;
}

void BlackRecogResultSendingSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void BlackRecogResultSendingSocket::slotSendBlackRecogReuslts(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults,QVector<LOG_ITEM>,int)
{
    if(!blackRecogResults.size())
        return;

    if(isRunning())
        return;

    m_blackRecogResults = blackRecogResults;
    start();
}

void BlackRecogResultSendingSocket::run()
{
    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    int blackRecogResultCount = m_blackRecogResults.size();
    sendingDataStream << blackRecogResultCount;
    for(int i = 0; i < blackRecogResultCount; i ++)
    {
        int canidateCount = m_blackRecogResults[i].size();
        sendingDataStream << canidateCount;

        for(int j = 0; j < canidateCount; j ++)
        {
            sendingDataStream << m_blackRecogResults[i][j].name;
            sendingDataStream << m_blackRecogResults[i][j].gender;
            sendingDataStream << m_blackRecogResults[i][j].birthday;
            sendingDataStream << m_blackRecogResults[i][j].address;
            sendingDataStream << m_blackRecogResults[i][j].description;
            sendingDataStream << m_blackRecogResults[i][j].personType;
            sendingDataStream << m_blackRecogResults[i][j].galleryFaceData;
            sendingDataStream << m_blackRecogResults[i][j].probeFaceData;
            sendingDataStream << m_blackRecogResults[i][j].similiarity;
            sendingDataStream << m_blackRecogResults[i][j].logUID;
            sendingDataStream << m_blackRecogResults[i][j].dateTime;
        }
    }

    sendingBuffer.close();


    int ret = sendDataBySocket(m_socket, sendingData);
    if(!ret)
    {
        closesocket(m_socket);
        socketError(this);
    }
}
