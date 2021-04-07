#include "logcheckthread.h"

#include <QtWidgets>

//#define _TEST_

void removeAllDirContents(QString dirNameStr);

LogCheckThread::LogCheckThread(QObject *parent) :
    QThread(parent)
{
    m_running = 0;
}

void LogCheckThread::startThread(QString logInfoName)
{
    m_logItemInfoName = logInfoName;

    QFile dataFile(m_logItemInfoName);

    bool opened = dataFile.open(QIODevice::ReadOnly);
    if(opened == false)
    {
        start();
        return;
    }

    m_logSavedDate.clear();
    QDataStream dataStream(&dataFile);

    dataStream >> m_logSavedDate;

    dataFile.close();

    start();
}

void LogCheckThread::stopThread()
{
    m_mutex.lock();
    m_running = 0;
    m_mutex.unlock();
    wait();

    QFile dataFile(m_logItemInfoName);

    bool opened = dataFile.open(QIODevice::WriteOnly);
    if(opened == false)
        return;

    QDataStream dataStream(&dataFile);
    dataStream << m_logSavedDate;
    dataFile.close();
}

QVector<QDate> LogCheckThread::logSavedDate()
{
    m_logMutex.lock();
    QVector<QDate> logSavedDate = m_tmpLogSavedDate;
    m_logMutex.unlock();
    return logSavedDate;
}


int getFreeSpace(QString drive)
{
    DWORD dwSectorsPerCluster, dwBytesPerSector,dwNumberOfFreeClusters, dwTotalNumberOfClusters;
    GetDiskFreeSpace((LPCWSTR)drive.utf16(), &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters);

#ifndef _TEST_
    int freeSpace = ((qint64)dwNumberOfFreeClusters * dwSectorsPerCluster * dwBytesPerSector) / ((qint64)1024 * 1024 * 1024);
    if(freeSpace >= 0)
        return freeSpace - 10;
    else if(freeSpace < 10 && freeSpace > 2)
        return 1;
    else
        qApp->quit();
#else
    int freeSpace = ((qint64)dwNumberOfFreeClusters * dwSectorsPerCluster * dwBytesPerSector) / ((qint64)1024 * 1024 * 1024);
    return freeSpace - 150;
#endif
}

void LogCheckThread::run()
{
    QString logDBPath = getDBPath();
    QDir dirInfo(logDBPath);
    QString abDBPath = dirInfo.absolutePath();

    char szDrive[256] = { 0 };
    _splitpath(abDBPath.toUtf8().data(), szDrive, NULL, NULL, NULL);

    QString drivePath = QString::fromUtf8(szDrive) + "\\";

    m_running = 1;
    while(1)
    {
        m_mutex.lock();
        int running = m_running;
        m_mutex.unlock();

        if(running == 0)
            break;

        m_logMutex.lock();
        QDate newDate = QDate::currentDate();
        m_tmpLogSavedDate = m_logSavedDate;
        m_logMutex.unlock();

        if(newDate.isValid())
        {
            int exist = -1;
            for(int i = 0; i < m_logSavedDate.size(); i ++)
            {
                if(m_logSavedDate[i] == newDate)
                {
                    exist = i;
                    break;
                }
            }

            if(exist < 0)
            {
                m_logSavedDate.append(newDate);

                QFile dataFile(m_logItemInfoName);

                bool opened = dataFile.open(QIODevice::WriteOnly);
                if(opened == false)
                    return;

                QDataStream dataStream(&dataFile);
                dataStream << m_logSavedDate;
                dataFile.close();
            }
        }

        int freeSpace = getFreeSpace(drivePath);
        int index = 0;
        while(freeSpace <= 0 && index < m_logSavedDate.size())
        {
            QString dateDirStr = getDBPath() + m_logSavedDate[index].toString("yyyy-MM-dd");
            removeAllDirContents(dateDirStr);

            m_mutex.lock();
            int running = m_running;
            m_mutex.unlock();
            if(running == 0)
                break;

            index ++;
            freeSpace = getFreeSpace(drivePath);
        }
        QThread::msleep(30);
    }
}


void LogCheckThread::removeAllDirContents(QString dirNameStr)
{
//    QString dirPath = dirNameStr + "/";
//    QDir dirLog(dirPath);

//    QStringList entryList = dirLog.entryList(QStringList("*"));
//    for(int i = 0; i < entryList.size(); i ++)
//    {
//        m_mutex.lock();
//        int running = m_running;
//        m_mutex.unlock();

//        if(running == 0)
//            break;


//        QFileInfo fileInfo(dirPath + entryList[i]);
//        if(entryList[i] == "." || entryList[i] == "..")
//            continue;

//        if(fileInfo.isDir())
//            removeAllDirContents(dirPath + entryList[i]);
//        else
//        {
//            QString fileName = dirPath + entryList[i];
//            QFile::remove(fileName);
//        }
//    }
//    RemoveDirectory((LPCWSTR)dirNameStr.utf16());

    QString filterStr = dirNameStr + "/*";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((LPCWSTR)filterStr.utf16(), &findFileData);
    if(hFind == NULL)
        return;

    while(FindNextFile(hFind, &findFileData) != 0)
    {
        m_mutex.lock();
        int running = m_running;
        m_mutex.unlock();

        if(running == 0)
            break;

        QString subDirStr = QString::fromUtf16((ushort*)findFileData.cFileName);
        if(subDirStr == "." || subDirStr == "..")
            continue;

        if(subDirStr != "." && subDirStr != ".." && findFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            removeAllDirContents(dirNameStr + "/" + subDirStr);
        else
        {
            QString fileName = dirNameStr + "/" + QString::fromUtf16((ushort*)findFileData.cFileName);
            DeleteFile((LPCWSTR)fileName.utf16());
        }
    }

    FindClose(hFind);
    RemoveDirectory((LPCWSTR)dirNameStr.utf16());
}
