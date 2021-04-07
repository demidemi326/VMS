#include "sceneimagesendsocket.h"
#include "socketbase.h"

#include <QtWidgets>

SceneImageSendSocket::SceneImageSendSocket(QObject *parent) :
    QThread(parent)
{
}

void SceneImageSendSocket::setSocketInfo(SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;

    start();
}

void SceneImageSendSocket::stopSocket()
{
    closesocket(m_socket);

    wait();
}

void SceneImageSendSocket::run()
{
    QString logUIDStr;
    int logType;
    int ret = receiveStringBySocket(m_socket, logUIDStr);
    ret = receiveIntBySocket(m_socket, logType);
    if(ret && logType == 0)
    {
        QByteArray sceneData;
        QByteArray sceneImageData;
        QByteArray sceneFeatureData;
        QRect      sceneFaceRect;

        QStringList splitList = logUIDStr.split("_");
        if(splitList.size() == 3)
        {
            QDateTime curDateTime = QDateTime::fromMSecsSinceEpoch(splitList[0].toLongLong());
            int chanelIndex = splitList[1].toInt();
            QString dateStr = curDateTime.date().toString("yyyy-MM-dd");
            QString chanelStr = "chanel " + QString::number(chanelIndex);


            QString cacheLogInfoPath = getDBPath() + dateStr + "/" + chanelStr + "/" + logUIDStr + ".bin";
            QFile cacheLogInfoFile(cacheLogInfoPath);
            if(!cacheLogInfoFile.open(QIODevice::ReadOnly))
            {
                QString logMetaInfoStr, logInfoStr;
                logMetaInfoStr.sprintf("%s/%s/%s/%d_LogMetaInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), curDateTime.time().hour());
                logInfoStr.sprintf("%s/%s/%s/%d_LogInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), curDateTime.time().hour());

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

                        if(logUID == logUIDStr)
                            break;
                    }

                    if(!logUID.isEmpty())
                    {
                        QFile logInfoFile(logInfoStr);
                        if(logInfoFile.open(QIODevice::ReadOnly))
        {
                            logInfoFile.seek(fileOffset);

                            QDataStream logInfoStream(&logInfoFile);

                            LOG_ITEM logItem;
                            QByteArray thumbFaceData;
                            logInfoStream >> logItem.logUID;
                            logInfoStream >> logItem.dateTime;
                            logInfoStream >> sceneFeatureData;
                            logInfoStream >> thumbFaceData;
                            logInfoStream >> sceneImageData;
                            logInfoStream >> sceneFaceRect;
                            logInfoFile.close();
                        }
                    }
                    logMetaInfoFile.close();
                }
            }
            else
            {
                QDataStream logInfoStream(&cacheLogInfoFile);

                LOG_ITEM logItem;
                QByteArray thumbFaceData;
                logInfoStream >> logItem.logUID;
                logInfoStream >> logItem.dateTime;
                logInfoStream >> sceneFeatureData;
                logInfoStream >> thumbFaceData;
                logInfoStream >> sceneImageData;
                logInfoStream >> sceneFaceRect;

                cacheLogInfoFile.close();
            }

            QBuffer sendingBuffer(&sceneData);
            sendingBuffer.open(QIODevice::WriteOnly);

            QDataStream sendingDataStream(&sendingBuffer);
            sendingDataStream << sceneImageData;
            sendingDataStream << sceneFeatureData;
            sendingDataStream << sceneFaceRect;
            sendingBuffer.close();
        }

        sendDataBySocket(m_socket, sceneData);
    }
    else if(ret && logType == 1)
    {
        QByteArray sceneData;
        QByteArray sceneImageData;
        QByteArray sceneFeatureData;
        QRect      sceneFaceRect;

        QStringList splitList = logUIDStr.split("_");
        if(splitList.size() == 3)
        {
            QDateTime curDateTime = QDateTime::fromMSecsSinceEpoch(splitList[0].toLongLong());
            int chanelIndex = splitList[1].toInt();
            QString dateStr = curDateTime.date().toString("yyyy-MM-dd");
            QString chanelStr = "chanel " + QString::number(chanelIndex);


            QString logMetaInfoStr, logInfoStr;
            logMetaInfoStr.sprintf("%s/%s/%s/%d_BlackMetaInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), curDateTime.time().hour());
            logInfoStr.sprintf("%s/%s/%s/%d_BlackInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), curDateTime.time().hour());

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

                    if(logUID == logUIDStr)
                        break;
                }

                if(!logUID.isEmpty())
                {
                    QFile logInfoFile(logInfoStr);
                    if(logInfoFile.open(QIODevice::ReadOnly))
                    {
                        logInfoFile.seek(fileOffset);
                        QDataStream blackInfoStream(&logInfoFile);

                        BLACK_RECOG_INFO blackRecogResult;
                        blackInfoStream >> blackRecogResult.name;
                        blackInfoStream >> blackRecogResult.gender;
                        blackInfoStream >> blackRecogResult.birthday;
                        blackInfoStream >> blackRecogResult.address;
                        blackInfoStream >> blackRecogResult.description;
                        blackInfoStream >> blackRecogResult.personType;
                        blackInfoStream >> blackRecogResult.galleryFaceData;
                        blackInfoStream >> blackRecogResult.probeFaceData;
                        blackInfoStream >> blackRecogResult.similiarity;
                        blackInfoStream >> sceneFeatureData;
                        blackInfoStream >> sceneImageData;
                        blackInfoStream >> sceneFaceRect;
                        blackInfoStream >> blackRecogResult.logUID;
                        blackInfoStream >> blackRecogResult.dateTime;

                        logInfoFile.close();
                    }
                }

                logMetaInfoFile.close();
            }

            QBuffer sendingBuffer(&sceneData);
            sendingBuffer.open(QIODevice::WriteOnly);

            QDataStream sendingDataStream(&sendingBuffer);
            sendingDataStream << sceneImageData;
            sendingDataStream << sceneFeatureData;
            sendingDataStream << sceneFaceRect;
            sendingBuffer.close();
        }

        sendDataBySocket(m_socket, sceneData);
    }

    closesocket(m_socket);
    emit sendFinshed(this);
}

