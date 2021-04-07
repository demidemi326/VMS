#include "searchlogsendingsocket.h"
#include "socketbase.h"
#include "servicebase.h"

#include <QtWidgets>

SearchLogSendingSocket::SearchLogSendingSocket(QObject *parent) :
    ChanelDataSendingSocket(parent)
{
}

void SearchLogSendingSocket::run()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
    {
        closesocket(m_socket);
        emit socketError(this);
        return;
    }

    QDateTime startTime, endTime;
    QByteArray probeFeatData;
    int threshold;
    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    QDataStream receiveDataStream(&receiveDataBuffer);
    receiveDataStream >> startTime;
    receiveDataStream >> endTime;
    receiveDataStream >> probeFeatData;
    receiveDataStream >> threshold;
    receiveDataBuffer.close();

    if(startTime > endTime)
    {
        closesocket(m_socket);
        emit socketError(this);
        return;
    }

    for(QDate curDate = startTime.date(); curDate <= endTime.date(); curDate = curDate.addDays(1))
    {
        QString dateStr = curDate.toString("yyyy-MM-dd");
        QString chanelStr = "chanel " + QString::number(m_chanelIndex);

        QString startTimeStr = QString::number(startTime.toMSecsSinceEpoch());
        QString endTimeStr = QString::number(endTime.toMSecsSinceEpoch());

        for(int i = 0; i < 24; i ++)
        {
            QString logMetaInfoStr, logInfoStr;
            logMetaInfoStr.sprintf("%s/%s/%s/%d_LogMetaInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), i);
            logInfoStr.sprintf("%s/%s/%s/%d_LogInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), i);

            LOG_ITEM logItem;
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

            if(spliterList[0] >= startTimeStr &&
                    spliterList[0] <= endTimeStr)
            {
                        QFile logInfoFile(logInfoStr);
                        if(logInfoFile.open(QIODevice::ReadOnly))
                {
                            logInfoFile.seek(fileOffset);

                            QDataStream logInfoStream(&logInfoFile);

                            QByteArray thumbFaceData, sceneData;
                            logInfoStream >> logItem.logUID;
                            logInfoStream >> logItem.dateTime;
                            logInfoStream >> logItem.featData;
                            logInfoStream >> thumbFaceData;
                            logInfoStream >> sceneData;
                            logInfoStream >> logItem.sceneFaceRect;
                            logInfoFile.close();

                            if(probeFeatData.size())
                            {
                                DistNo* distNos = IdentifyCPU(probeFeatData.data(), logItem.featData.data(), 1);
                    if((1 - distNos[0].rDist) * 100 < threshold)
                    {
                        EngineFree(distNos);
                        continue;
                    }
                    EngineFree(distNos);
                }

                QByteArray sendData;
                QBuffer sendingBuffer(&sendData);
                sendingBuffer.open(QIODevice::WriteOnly);

                QDataStream sendingDataStream(&sendingBuffer);

                            sendingDataStream << logItem.dateTime;
                            sendingDataStream << thumbFaceData;
                            sendingDataStream << logItem.logUID;
                sendingBuffer.close();

                int ret = sendDataBySocket(m_socket, sendData);
                if(!ret)
                {
                    closesocket(m_socket);
                    emit socketError(this);
                    return;
                }
            }
        }
    }

                logMetaInfoFile.close();
            }
        }
    }


    closesocket(m_socket);
    emit socketError(this);
}
