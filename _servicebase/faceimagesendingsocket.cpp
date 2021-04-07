#include "faceimagesendingsocket.h"
#include "socketbase.h"

#include <QtWidgets>

FaceImageSendingSocket::FaceImageSendingSocket(QObject *parent) :
    QThread(parent)
{
    m_chanelIndex = -1;
}

void FaceImageSendingSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;
}

void FaceImageSendingSocket::stopSocket()
{
    closesocket(m_socket);
    wait();
}

void FaceImageSendingSocket::slotSendFaceImages(QVector<QVector<BLACK_RECOG_INFO> >, QVector<LOG_ITEM> logItems, int)
{
    if(isRunning())
        return;

    m_logItem = logItems;
    start();
}

void FaceImageSendingSocket::run()
{
    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);
    sendingDataStream << m_logItem.size();
    for(int i = 0; i < m_logItem.size(); i ++)
    {
        sendingDataStream << m_logItem[i].dateTime;

        QImage capturedImage = m_logItem[i].capturedFace;

        QByteArray sendingCapturedImage;
        QBuffer capturedImageBuffer(&sendingCapturedImage);
        capturedImageBuffer.open(QIODevice::WriteOnly);
        capturedImage.save(&capturedImageBuffer, "JPG");
        capturedImageBuffer.close();

        sendingDataStream << sendingCapturedImage;

        sendingDataStream << m_logItem[i].logUID;

        for(int j = 0; j < 3; j ++)
        {
            QByteArray candidateFace = qImage2ByteArray(m_logItem[i].candidateFace[j]);
            sendingDataStream << candidateFace;
            sendingDataStream << m_logItem[i].candidateDist[j];
            sendingDataStream << m_logItem[i].candidateType[j];
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
