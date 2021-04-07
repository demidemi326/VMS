#include "imageprocessingsocket.h"
#include "socketbase.h"

#include <QtWidgets>

ImageProcessingSocket::ImageProcessingSocket(QObject *parent) :
    QThread(parent)
{

}

void ImageProcessingSocket::startProcessing(ServerInfo serverInfo, QImage image)
{
    stopProcessing();

    m_serverInfo = serverInfo;
    m_image = image;

    start();
}

void ImageProcessingSocket::stopProcessing()
{
    closesocket(m_socket);
    wait();
}

void ImageProcessingSocket::run()
{
    SOCKADDR_IN target; //Information about host
    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
        return;

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfo.port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
        return;

    int ret = authenticateSocket();
    if(ret)
    {
        sendProcessingImage();
        recvProcessingResult();
    }

    closesocket(m_socket);
}

int ImageProcessingSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "IMAGE_PROCESS");
        return 1;
    }
    else
        return 0;
}

int ImageProcessingSocket::sendProcessingImage()
{
    QByteArray sendingImageData;
    QBuffer sendingImageBuffer(&sendingImageData);
    sendingImageBuffer.open(QIODevice::WriteOnly);
    m_image.save(&sendingImageBuffer, "JPG");
    sendingImageBuffer.close();

    return sendDataBySocket(m_socket, sendingImageData);
}

int ImageProcessingSocket::recvProcessingResult()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
        return 0;

    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    int receiveFeatCount = 0;
    QDataStream receiveDataStream(&receiveDataBuffer);

    receiveDataStream >> receiveFeatCount;
    QVector<FRAME_RESULT>   frameResults;
    for(int i = 0; i < receiveFeatCount; i ++)
    {
        FRAME_RESULT frameResult;
        receiveDataStream >> frameResult.featData;
        receiveDataStream >> frameResult.faceRect;

        frameResult.faceImage = cropFaceImage(m_image, frameResult.faceRect);
        frameResults.append(frameResult);
    }

    receiveDataBuffer.close();

    emit receiveFrameResults(frameResults);

    return 1;
}
