#include "searchblacksendingsocket.h"
#include "socketbase.h"
#include "servicebase.h"

#include <QtWidgets>

SearchBlackSendingSocket::SearchBlackSendingSocket(QObject *parent) :
    ChanelDataSendingSocket(parent)
{
}

void SearchBlackSendingSocket::run()
{
    QByteArray receiveData;
    int ret = receiveDataBySocket(m_socket, receiveData);
    if(!ret)
    {
        emit socketError(this);
        return;
    }

    QDateTime startTime, endTime;
    QString searchName;
    QBuffer receiveDataBuffer(&receiveData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    QDataStream receiveDataStream(&receiveDataBuffer);
    receiveDataStream >> startTime;
    receiveDataStream >> endTime;
    receiveDataStream >> searchName;
    receiveDataBuffer.close();


    for(QDate curDate = startTime.date(); curDate <= endTime.date(); curDate = curDate.addDays(1))
    {
        QString dateStr = curDate.toString("yyyy-MM-dd");
        QString chanelStr = "chanel " + QString::number(m_chanelIndex);

        QString startTimeStr = QString::number(startTime.toMSecsSinceEpoch());
        QString endTimeStr = QString::number(endTime.toMSecsSinceEpoch());

        for(int i = 0; i < 24; i ++)
        {
            QString logMetaInfoStr, logInfoStr;
            logMetaInfoStr.sprintf("%s/%s/%s/%d_BlackMetaInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), i);
            logInfoStr.sprintf("%s/%s/%s/%d_BlackInfo.bin", getDBPath().toUtf8().data(), dateStr.toUtf8().data(), chanelStr.toUtf8().data(), i);

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

            if(spliterList[0] >= startTimeStr &&
                spliterList[0] <= endTimeStr)
            {
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

                            if(!blackRecogResult.name.contains(searchName))
                    continue;

                QByteArray sendData;
                QBuffer sendingBuffer(&sendData);
                sendingBuffer.open(QIODevice::WriteOnly);

                QDataStream sendingDataStream(&sendingBuffer);

                sendingDataStream << (int)1;
                            sendingDataStream << (int)1;
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

//        QString blackLogPath = getDBPath() + dateStr + "/" + chanelStr + "/_blackResults/";
//        QDir blackLogDir(blackLogPath);
//        QString nameFilter;
//        nameFilter.sprintf("*.bin");

//        QStringList entryList = blackLogDir.entryList(QStringList(nameFilter), QDir::Files, QDir::Name);
//        QString startTimeStr = QString::number(startTime.toMSecsSinceEpoch());
//        QString endTimeStr = QString::number(endTime.toMSecsSinceEpoch());

//        for(int i = 0; i < entryList.size(); i ++)
//        {
//            QStringList spliterList = entryList[i].split("_");
//            if(spliterList.size() != 3)
//                continue;

//            if(spliterList[0] >= startTimeStr &&
//                spliterList[0] <= endTimeStr)
//            {
//                QByteArray blackData;
//                QFile blackFile(blackLogPath + entryList[i]);
//                if(!blackFile.open(QIODevice::ReadOnly))
//                    continue;

//                blackData = blackFile.readAll();
//                blackFile.close();

//                QBuffer blackBuffer(&blackData);
//                blackBuffer.open(QIODevice::ReadOnly);
//                QDataStream blackBufferStream(&blackBuffer);

//                QString blackName;
//                blackBufferStream >> blackName;
//                blackBuffer.close();

//                if(!blackName.contains(searchName))
//                    continue;

//                QByteArray sendData;
//                QBuffer sendingBuffer(&sendData);
//                sendingBuffer.open(QIODevice::WriteOnly);

//                QDataStream sendingDataStream(&sendingBuffer);

//                sendingDataStream << (int)1;
//                sendingDataStream << blackData;
//                sendingBuffer.close();

//                int ret = sendDataBySocket(m_socket, sendData);
//                if(!ret)
//                {
//                    closesocket(m_socket);
//                    emit socketError(this);

//                    return;
//                }
//            }
//        }
    }

    closesocket(m_socket);
    emit socketError(this);
}
