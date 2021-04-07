#include "logsavethread.h"

#include <QtWidgets>

LogSaveThread::LogSaveThread(QObject *parent) :
    QThread(parent)
{
}

void LogSaveThread::startSaveLog(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults, QVector<LOG_ITEM> logItems, int chanelIndex)
{
    m_blackRecogResults = blackRecogResults;
    m_logItems = logItems;
    m_chanelIndex = chanelIndex;

    start();
}

void LogSaveThread::run()
{
    for(int i = 0; i < m_logItems.size(); i ++)
    {
        LOG_ITEM logItem = m_logItems[i];
    QDateTime curDateTime = QDateTime::currentDateTime();
        curDateTime = getDateTimeFromUID(logItem.logUID);

    QDir rootDir(getDBPath());

    QString dateStr = curDateTime.date().toString("yyyy-MM-dd");
    rootDir.mkdir(dateStr);

    QDir curDateDir(rootDir.absolutePath() + "/" + dateStr);
    QString chanelStr = "chanel " + QString::number(m_chanelIndex);
    curDateDir.mkdir(chanelStr);

        QString chanelDirPath = curDateDir.absolutePath() + "/" +  chanelStr;

        if(logItem.isFinished)
        {
            QString logMetaInfoStr, logInfoStr, oldLogInfoStr;
            logMetaInfoStr.sprintf("%s/%d_LogMetaInfo.bin", chanelDirPath.toUtf8().data(), curDateTime.time().hour());
            logInfoStr.sprintf("%s/%d_LogInfo.bin", chanelDirPath.toUtf8().data(), curDateTime.time().hour());
            oldLogInfoStr.sprintf("%s/%s.bin", chanelDirPath.toUtf8().data(), logItem.logUID.toUtf8().data());

            QFile::remove(oldLogInfoStr);

            QFile logInfoFile(logInfoStr);
            QFile logMetaInfoFile(logMetaInfoStr);
            logInfoFile.open(QIODevice::Append);
            qint64 fileOffset = logInfoFile.size();

            QDataStream logInfoStream(&logInfoFile);
            logInfoStream << logItem.logUID;
            logInfoStream << logItem.dateTime;
            logInfoStream << logItem.featData;
            logInfoStream << qImage2ByteArray(logItem.capturedFace);
            logInfoStream << qImage2ByteArray(logItem.sceneImage);
            logInfoStream << logItem.sceneFaceRect;

            logInfoFile.close();


            logMetaInfoFile.open(QIODevice::Append);
            QDataStream logMetaInfoStream(&logMetaInfoFile);

            logMetaInfoStream << logItem.logUID;
            logMetaInfoStream << fileOffset;

            logMetaInfoFile.close();
        }
        else
        {
            QString logInfoStr;
            logInfoStr.sprintf("%s/%s.bin", chanelDirPath.toUtf8().data(), logItem.logUID.toUtf8().data());

            QFile logInfoFile(logInfoStr);
            logInfoFile.open(QIODevice::WriteOnly);

            QDataStream logInfoStream(&logInfoFile);
            logInfoStream << logItem.logUID;
            logInfoStream << logItem.dateTime;
            logInfoStream << logItem.featData;
            logInfoStream << qImage2ByteArray(logItem.capturedFace);
            logInfoStream << qImage2ByteArray(logItem.sceneImage);
            logInfoStream << logItem.sceneFaceRect;

            logInfoFile.close();
        }
    }

    for(int i = 0; i < m_blackRecogResults.size(); i ++)
    {
        BLACK_RECOG_INFO blackRecogResult = m_blackRecogResults[i][0];
        QDateTime curDateTime = QDateTime::currentDateTime();
        curDateTime = getDateTimeFromUID(blackRecogResult.logUID);

        QDir rootDir(getDBPath());

        QString dateStr = curDateTime.date().toString("yyyy-MM-dd");
        rootDir.mkdir(dateStr);

        QDir curDateDir(rootDir.absolutePath() + "/" + dateStr);
        QString chanelStr = "chanel " + QString::number(m_chanelIndex);
        curDateDir.mkdir(chanelStr);

        QString chanelDirPath = curDateDir.absolutePath() + "/" +  chanelStr;

        QString blackMetaInfoStr, blackInfoStr;
        blackMetaInfoStr.sprintf("%s/%d_BlackMetaInfo.bin", chanelDirPath.toUtf8().data(), curDateTime.time().hour());
        blackInfoStr.sprintf("%s/%d_BlackInfo.bin", chanelDirPath.toUtf8().data(), curDateTime.time().hour());

        QFile blackInfoFile(blackInfoStr);
        QFile blackMetaInfoFile(blackMetaInfoStr);
        blackInfoFile.open(QIODevice::Append);
        qint64 fileOffset = blackInfoFile.size();

        QDataStream blackInfoStream(&blackInfoFile);
        blackInfoStream << blackRecogResult.name;
        blackInfoStream << blackRecogResult.gender;
        blackInfoStream << blackRecogResult.birthday;
        blackInfoStream << blackRecogResult.address;
        blackInfoStream << blackRecogResult.description;
        blackInfoStream << blackRecogResult.personType;
        blackInfoStream << blackRecogResult.galleryFaceData;
        blackInfoStream << blackRecogResult.probeFaceData;
        blackInfoStream << blackRecogResult.similiarity;
        blackInfoStream << blackRecogResult.featData;
        blackInfoStream << qImage2ByteArray(blackRecogResult.sceneImage);
        blackInfoStream << blackRecogResult.sceneFaceRect;
        blackInfoStream << blackRecogResult.logUID;
        blackInfoStream << blackRecogResult.dateTime;

        blackInfoFile.close();

        blackMetaInfoFile.open(QIODevice::Append);
        QDataStream logMetaInfoStream(&blackMetaInfoFile);

        logMetaInfoStream << blackRecogResult.logUID;
        logMetaInfoStream << fileOffset;

        blackMetaInfoFile.close();
    }

    emit savedLogInfo(QDate::currentDate());
    emit saveFinished(this);
}
