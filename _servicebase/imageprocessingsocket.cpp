#include "imageprocessingsocket.h"
#include "socketbase.h"
#include "servicebase.h"

#include <QtWidgets>

ImageProcessingSocket::ImageProcessingSocket(QObject *parent) :
    QThread(parent)
{
    m_chanelIndex = -1;
}

void ImageProcessingSocket::setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;
    m_chanelIndex = chanelIndex;

    start();
}

void ImageProcessingSocket::stopSocket()
{
    closesocket(m_socket);

    wait();
}

void ImageProcessingSocket::run()
{
    int ret = receiveImage();
    if(!ret)
    {
        processingFinshed(this);
        return;
    }

    ret = procssingImage();
    if(!ret)
    {
        processingFinshed(this);
        return;
    }

    sendResults();

    processingFinshed(this);
}

int ImageProcessingSocket::receiveImage()
{
    QByteArray recvImageData;
    int ret = receiveDataBySocket(m_socket, recvImageData);
    if(!ret)
        return 0;

    m_image = QImage::fromData(recvImageData, "JPG");
    return 1;
}

int ImageProcessingSocket::procssingImage()
{
    if(m_image.isNull() || m_chanelIndex < 0)
        return 0;

    SImg srcFrame = qImage2SImg(m_image);
    int faceCount;
    DetectionResult* srcDetectionResults = FaceDetection(&srcFrame, 0, &faceCount);
    if(faceCount < 0)
    {
        qApp->quit();
        return 0;
    }

    if(faceCount > 0)
    {
        MFResult* mfResults = MFExtraction(&srcFrame, srcDetectionResults, faceCount);

        m_frameResults.clear();
        for(int i = 0; i < faceCount; i ++)
        {
            SENDING_FRAME_RESULT frameResult;
            frameResult.featData = QByteArray((char*)&mfResults[i].xFeature, sizeof(ARM_Feature));
            frameResult.faceRect = QRect((int)srcDetectionResults[i].xFaceRect.rX, (int)srcDetectionResults[i].xFaceRect.rY,
                                         (int)(28 * srcDetectionResults[i].xFaceRect.m_rRate), (int)(28 * srcDetectionResults[i].xFaceRect.m_rRate));

            m_frameResults.append(frameResult);
        }

        EngineFree(mfResults);
        EngineFree(srcDetectionResults);
    }

    freeSImg(&srcFrame);

    return 1;
}

int ImageProcessingSocket::sendResults()
{
    QByteArray sendingData;
    QBuffer sendingBuffer(&sendingData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);
    sendingDataStream << m_frameResults.size();
    for(int i = 0; i < m_frameResults.size(); i ++)
    {
        sendingDataStream << m_frameResults[i].featData;
        sendingDataStream << m_frameResults[i].faceRect;
    }

    sendingBuffer.close();

    return sendDataBySocket(m_socket, sendingData);
}
