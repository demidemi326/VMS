#include "searchlastblacksendingsocket.h"
#include "socketbase.h"
#include "servicebase.h"
#include <QtWidgets>

SearchLastBlackSendingSocket::SearchLastBlackSendingSocket(QObject *parent) :
    ChanelDataSendingSocket(parent)
{
}

void SearchLastBlackSendingSocket::setSavedLogDate(QVector<QDate> logSavedDate)
{
    m_logSavedDate = logSavedDate;
}

void SearchLastBlackSendingSocket::run()
{
#define N_MAX_BLACKSIZE 100
    QVector<QByteArray> sendingBlackData;
    for(int i = m_logSavedDate.size() - 1; i >= 0; i --)
    {
        QDate curDate = m_logSavedDate[i];
        QString dateStr = curDate.toString("yyyy-MM-dd");
        QString chanelStr = "chanel " + QString::number(m_chanelIndex);

        for(int j = 23; j >= 0; j --)
        {
            QVector<QByteArray> tmpBlackData;
            QString logMetaInfoStr, logInfoStr;
            logMetaInfoStr.sprintf("%s/%s/%s/%d_BlackMetaInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), j);
            logInfoStr.sprintf("%s/%s/%s/%d_BlackInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), j);

            BLACK_RECOG_INFO blackRecogResult;
            QFile logMetaInfoFile(logMetaInfoStr);

            if(logMetaInfoFile.open(QIODevice::ReadOnly))
            {
                QDataStream logMetaInfoStream(&logMetaInfoFile);

                QString logUID;
                qint64 fileOffset;

                while(1)
        {
                    logMetaInfoStream >> logUID;
                    logMetaInfoStream >> fileOffset;

                    if(logUID.isEmpty())
                        break;


                    QStringList spliterList = logUID.split("_");
            if(spliterList.size() != 3)
                continue;

                    QFile logInfoFile(logInfoStr);
                    if(logInfoFile.open(QIODevice::ReadOnly))
                    {
                        logInfoFile.seek(fileOffset);

                        QDataStream blackInfoStream(&logInfoFile);

                        QByteArray sceneData;
                        blackInfoStream >> blackRecogResult.name;
                        blackInfoStream >> blackRecogResult.gender;
                        blackInfoStream >> blackRecogResult.birthday;
                        blackInfoStream >> blackRecogResult.address;
                        blackInfoStream >> blackRecogResult.description;
                        blackInfoStream >> blackRecogResult.personType;
                        blackInfoStream >> blackRecogResult.galleryFaceData;
                        blackInfoStream >> blackRecogResult.probeFaceData;
                        blackInfoStream >> blackRecogResult.similiarity;
                        blackInfoStream >> blackRecogResult.featData;
                        blackInfoStream >> sceneData;
                        blackInfoStream >> blackRecogResult.sceneFaceRect;
                        blackInfoStream >> blackRecogResult.logUID;
                        blackInfoStream >> blackRecogResult.dateTime;
                        logInfoFile.close();

                        QByteArray sendData;
                        QBuffer sendingBuffer(&sendData);
                        sendingBuffer.open(QIODevice::WriteOnly);

                        QDataStream sendingDataStream(&sendingBuffer);

                        sendingDataStream << blackRecogResult.name;
                        sendingDataStream << blackRecogResult.gender;
                        sendingDataStream << blackRecogResult.birthday;
                        sendingDataStream << blackRecogResult.address;
                        sendingDataStream << blackRecogResult.description;
                        sendingDataStream << blackRecogResult.personType;
                        sendingDataStream << blackRecogResult.galleryFaceData;
                        sendingDataStream << blackRecogResult.probeFaceData;
                        sendingDataStream << blackRecogResult.similiarity;
                        sendingDataStream << blackRecogResult.logUID;
                        sendingDataStream << blackRecogResult.dateTime;
                        sendingBuffer.close();

                        tmpBlackData.append(sendData);

                        if(tmpBlackData.size() > N_MAX_BLACKSIZE)
                            tmpBlackData.remove(0);
                    }
                }

                logMetaInfoFile.close();
            }

            tmpBlackData += sendingBlackData;
            sendingBlackData = tmpBlackData;
            if(sendingBlackData.size() > N_MAX_BLACKSIZE)
                break;
        }

        if(sendingBlackData.size() > N_MAX_BLACKSIZE)
            break;
    }

    QByteArray sendData;
    QBuffer sendingBuffer(&sendData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << sendingBlackData;
    sendingBuffer.close();

    sendDataBySocket(m_socket, sendData);
    closesocket(m_socket);
    emit socketError(this);
}

