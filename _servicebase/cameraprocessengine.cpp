#include "cameraprocessengine.h"

#include "servicebase.h"
#include "socketbase.h"
#include "cluster.h"
#include "ipcamera.h"

#include <QtWidgets>
#include <process.h>

#ifdef _CREATE_THREAD_
DWORD faceDetectionProc(LPVOID param);
DWORD modelDetectionProc(LPVOID param);
#else
void faceDetectionProc(void* param);
void modelDetectionProc(void* param);
void cameraLoginProc(void* param);
#endif

#define N_MAX_FEATURE_NUM 50

void convertYUV420P_toRGB888(unsigned char* data, int width, int height, unsigned char* dstData);

CameraProcessEngine::CameraProcessEngine(QObject *parent) :
    QObject(parent)
{
    m_chanelIndex = -1;

    m_faceDetectionRunning = 0;
    m_modelDetectionRunning = 0;

    m_faceDetectionImageTime = 0;
    m_modelDetectionImageTime = 0;

    m_faceDetectionThread = NULL;
    m_modelDetectionThread = NULL;

    m_identifyThreshold = 0;
    m_blackCandidateCount = 0;

    memset(&m_blackPersonFeats, 0, sizeof(PersonDatabase1));

    m_ipCamera = new IpCamera;
    qRegisterMetaType<YUVFrame>("YUVFrame");
}

CameraProcessEngine::~CameraProcessEngine()
{
    delete m_ipCamera;
}

void CameraProcessEngine::startProcess(int chanelIndex, IpCameraInfo cameraInfo)
{
    stopProcess();

    m_cameraInfo = cameraInfo;
    m_chanelIndex = chanelIndex;

    loadBlackList(chanelIndex);

    m_ipCamera->setReconnect(1);
    m_ipCamera->open(cameraInfo.streamUri);

#ifndef _CREATE_THREAD_
    m_faceDetectionThread = (HANDLE)_beginthread(faceDetectionProc, 0, this);
    m_modelDetectionThread = (HANDLE)_beginthread(modelDetectionProc, 0, this);
#else
    m_faceDetectionThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&faceDetectionProc,(LPVOID)this, 0, NULL);
    m_modelDetectionThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&modelDetectionProc,(LPVOID)this, 0, NULL);
#endif

    SetThreadPriority(m_faceDetectionThread, THREAD_PRIORITY_HIGHEST);
    SetThreadPriority(m_modelDetectionThread, THREAD_PRIORITY_NORMAL);
}

void CameraProcessEngine::stopProcess()
{
    if(m_modelDetectionRunning)
    {
        setModelDetectionRunning(0);
        WaitForSingleObject(m_modelDetectionThread, 5000);
        m_modelDetectionThread = NULL;
    }
    else
        m_modelDetectionThread = NULL;

    if(m_faceDetectionRunning)
    {
        setFaceDetectionRunning(0);
        WaitForSingleObject(m_faceDetectionThread, 5000);
        m_faceDetectionThread = NULL;
    }
    else
        m_faceDetectionThread = NULL;

    m_ipCamera->setReconnect(0);
    m_ipCamera->close();

    m_faceRectSendingMutex.lock();
    for(int i = 0; i < m_faceRectSendingSockets.size(); i ++)
        closesocket(m_faceRectSendingSockets[i]);
    m_faceRectSendingSockets.clear();
    m_faceRectSendingMutex.unlock();

    m_logSendingMutex.lock();
    for(int i = 0; i < m_logSendingSockets.size(); i ++)
        closesocket(m_logSendingSockets[i]);
    m_logSendingSockets.clear();
    m_logSendingMutex.unlock();

    m_blackResultSendingMutex.lock();
    for(int i = 0; i < m_blackResultSendingSockets.size(); i ++)
        closesocket(m_blackResultSendingSockets[i]);
    m_blackResultSendingSockets.clear();
    m_blackResultSendingMutex.unlock();

    if(m_chanelIndex >= 0)
    {
        saveBlackList(m_chanelIndex);
        freeAllPersonDatabase1(&m_blackPersonFeats);
        m_blackPersonInfos.clear();
    }

    m_chanelIndex = -1;
}

int CameraProcessEngine::status()
{
    return m_ipCamera->isRunning();
}

IpCameraInfo CameraProcessEngine::cameraInfos()
{
    QMutexLocker locker(&m_settingMutex);

    IpCameraInfo cameraInfo = m_cameraInfo;
    return cameraInfo;
}

void CameraProcessEngine::setIpCameraInfo(IpCameraInfo cameraInfo)
{
    QMutexLocker locker(&m_settingMutex);

    m_cameraInfo.ipAddress = cameraInfo.ipAddress;
    m_cameraInfo.portNum = cameraInfo.portNum;
    m_cameraInfo.videoSource = cameraInfo.videoSource;
    m_cameraInfo.streamUri = cameraInfo.streamUri;
}

void CameraProcessEngine::setSurveillanceSetting(IpCameraInfo cameraInfo)
{
    QMutexLocker locker(&m_settingMutex);

    m_cameraInfo.chkFaceSize = cameraInfo.chkFaceSize;
    m_cameraInfo.detectionFaceMinSize = cameraInfo.detectionFaceMinSize;
    m_cameraInfo.chkThreshold = cameraInfo.chkThreshold;
    m_cameraInfo.identifyThreshold = cameraInfo.identifyThreshold;
    m_cameraInfo.chkBlackCanidiateCount = cameraInfo.chkBlackCanidiateCount;
    m_cameraInfo.blackCandidateCount = cameraInfo.blackCandidateCount;
}

void CameraProcessEngine::addFaceRectSendingSocket(SOCKET socket)
{
    QMutexLocker locker(&m_faceRectSendingMutex);

    m_faceRectSendingSockets.append(socket);
}

void CameraProcessEngine::addLogSendingSocket(SOCKET socket)
{
    QMutexLocker locker(&m_logSendingMutex);

    m_logSendingSockets.append(socket);
}

void CameraProcessEngine::addBlackSendingSocket(SOCKET socket)
{
    QMutexLocker locker(&m_blackResultSendingMutex);

    m_blackResultSendingSockets.append(socket);
}

int CameraProcessEngine::chanelIndex()
{
    return m_chanelIndex;
}

QVector<BlackPersonMetaInfo> CameraProcessEngine::blackPersonMeataInfo()
{
    QMutexLocker locker(&m_personMutex);

    return m_blackPersonInfos;
}

PersonDatabase1 CameraProcessEngine::blackPersonDatabase()
{
    QMutexLocker locker(&m_personMutex);

    return m_blackPersonFeats;
}

void CameraProcessEngine::slotAddOneBlackPerson(Person1 personFeat,BlackPersonMetaInfo personInfo)
{
    QMutexLocker locker(&m_personMutex);

    int exist = -1;
    for(int i = 0; i < m_blackPersonInfos.size(); i ++)
    {
        if(m_blackPersonInfos[i].name == personInfo.name)
        {
            exist = i;
            break;
        }
    }
    if(exist >= 0)
    {
        Person1 oldPersonFeat = m_blackPersonFeats.pxPersons[exist];
        Person1 newPersonFeat = { 0 };
        newPersonFeat.nFeatureNum = oldPersonFeat.nFeatureNum + personFeat.nFeatureNum;
        newPersonFeat.pxFeatures = (ARM_Feature*)malloc(sizeof(ARM_Feature) * newPersonFeat.nFeatureNum);
        memcpy(newPersonFeat.pxFeatures, oldPersonFeat.pxFeatures, oldPersonFeat.nFeatureNum * sizeof(ARM_Feature));
        memcpy(newPersonFeat.pxFeatures + oldPersonFeat.nFeatureNum, personFeat.pxFeatures, personFeat.nFeatureNum * sizeof(ARM_Feature));

        freePerson1(personFeat);
        freePerson1(oldPersonFeat);
        m_blackPersonFeats.pxPersons[exist] = newPersonFeat;
        m_blackPersonInfos[exist].gender = personInfo.gender;
        m_blackPersonInfos[exist].birthday = personInfo.birthday;
        m_blackPersonInfos[exist].address = personInfo.address;
        m_blackPersonInfos[exist].description = personInfo.description;
        m_blackPersonInfos[exist].personType = personInfo.personType;

        for(int i = 0; i < personInfo.faceData.size(); i ++)
            m_blackPersonInfos[exist].faceData.append(personInfo.faceData[i]);
    }
    else
    {
        PersonDatabase1 newBlackPersonFeats;
        newBlackPersonFeats.nPersonNum = m_blackPersonFeats.nPersonNum + 1;
        newBlackPersonFeats.pxPersons = (Person1*)malloc(sizeof(Person1) * newBlackPersonFeats.nPersonNum);
        if(m_blackPersonFeats.nPersonNum)
            memcpy(newBlackPersonFeats.pxPersons, m_blackPersonFeats.pxPersons, m_blackPersonFeats.nPersonNum * sizeof(Person1));

        memcpy(newBlackPersonFeats.pxPersons + m_blackPersonFeats.nPersonNum, &personFeat, sizeof(Person1));

        freePersonDatabase1(m_blackPersonFeats);
        m_blackPersonFeats = newBlackPersonFeats;
        m_blackPersonInfos.append(personInfo);
    }

    saveBlackList(m_chanelIndex);
}

void CameraProcessEngine::slotAddBlackPersonData(QByteArray blackPersonData)
{
    QMutexLocker locker(&m_personMutex);

    QBuffer receiveDataBuffer(&blackPersonData);
    receiveDataBuffer.open(QIODevice::ReadOnly);

    QDataStream receiveDataStream(&receiveDataBuffer);

    int blackInfoCount = 0;
    receiveDataStream >> blackInfoCount;

    for(int i = 0; i < blackInfoCount; i ++)
    {
        QVector<QByteArray> featData;
        receiveDataStream >> featData;

        BlackPersonMetaInfo personMetaInfo;
        receiveDataStream >> personMetaInfo.name;
        receiveDataStream >> personMetaInfo.gender;
        receiveDataStream >> personMetaInfo.birthday;
        receiveDataStream >> personMetaInfo.address;
        receiveDataStream >> personMetaInfo.description;
        receiveDataStream >> personMetaInfo.personType;
        receiveDataStream >> personMetaInfo.faceData;

        int exist = -1;
        for(int j = 0; j < m_blackPersonInfos.size(); j ++)
        {
            if(m_blackPersonInfos[j].name == personMetaInfo.name)
            {
                exist = j;
                break;
            }
        }

        if(exist >= 0)
        {
            Person1 oldPersonFeat = m_blackPersonFeats.pxPersons[exist];
            Person1 newPersonFeat = { 0 };
            newPersonFeat.nFeatureNum = oldPersonFeat.nFeatureNum + featData.size();
            newPersonFeat.pxFeatures = (ARM_Feature*)malloc(sizeof(ARM_Feature) * newPersonFeat.nFeatureNum);
            memcpy(newPersonFeat.pxFeatures, oldPersonFeat.pxFeatures, oldPersonFeat.nFeatureNum * sizeof(ARM_Feature));
            for(int j = 0; j < featData.size(); j ++)
                memcpy(newPersonFeat.pxFeatures + oldPersonFeat.nFeatureNum + j, featData.data(), sizeof(ARM_Feature));

            freePerson1(oldPersonFeat);
            m_blackPersonFeats.pxPersons[exist] = newPersonFeat;
            m_blackPersonInfos[exist].gender = personMetaInfo.gender;
            m_blackPersonInfos[exist].birthday = personMetaInfo.birthday;
            m_blackPersonInfos[exist].address = personMetaInfo.address;
            m_blackPersonInfos[exist].description = personMetaInfo.description;
            m_blackPersonInfos[exist].personType = personMetaInfo.personType;

            for(int i = 0; i < personMetaInfo.faceData.size(); i ++)
                m_blackPersonInfos[exist].faceData.append(personMetaInfo.faceData[i]);
        }
        else
        {
            Person1 personFeat;
            personFeat.nFeatureNum = featData.size();

            personFeat.pxFeatures = (ARM_Feature*)malloc(sizeof(ARM_Feature) * personFeat.nFeatureNum);
            for(int j = 0; j < personFeat.nFeatureNum; j ++)
                memcpy(&personFeat.pxFeatures[j], featData[j].data(), featData[j].size());


            PersonDatabase1 newBlackPersonFeats;
            newBlackPersonFeats.nPersonNum = m_blackPersonFeats.nPersonNum + 1;
            newBlackPersonFeats.pxPersons = (Person1*)malloc(sizeof(Person1) * newBlackPersonFeats.nPersonNum);
            if(m_blackPersonFeats.nPersonNum)
                memcpy(newBlackPersonFeats.pxPersons, m_blackPersonFeats.pxPersons, m_blackPersonFeats.nPersonNum * sizeof(Person1));

            memcpy(newBlackPersonFeats.pxPersons + m_blackPersonFeats.nPersonNum, &personFeat, sizeof(Person1));

            freePersonDatabase1(m_blackPersonFeats);
            m_blackPersonFeats = newBlackPersonFeats;
            m_blackPersonInfos.append(personMetaInfo);
        }
    }

    receiveDataBuffer.close();

    saveBlackList(m_chanelIndex);
}

void CameraProcessEngine::slotEditOneBlackPerson(QString oldName, Person1 personFeat, BlackPersonMetaInfo personInfo)
{
    QMutexLocker locker(&m_personMutex);

    for(int i = 0; i < m_blackPersonInfos.size(); i ++)
    {
        if(m_blackPersonInfos[i].name == oldName)
        {
            if(m_blackPersonFeats.pxPersons[i].pxFeatures)
                free(m_blackPersonFeats.pxPersons[i].pxFeatures);

            m_blackPersonFeats.pxPersons[i] = personFeat;

            m_blackPersonInfos[i].name = personInfo.name;
            m_blackPersonInfos[i].address = personInfo.address;
            m_blackPersonInfos[i].birthday = personInfo.birthday;
            m_blackPersonInfos[i].description = personInfo.description;
            m_blackPersonInfos[i].personType = personInfo.personType;
            m_blackPersonInfos[i].gender = personInfo.gender;

            m_blackPersonInfos[i].faceData = personInfo.faceData;
        }
    }
    saveBlackList(m_chanelIndex);

    qDebug() << "edit black info" << personInfo.name;
}

void CameraProcessEngine::slotDeleteBlackInfos(QStringList deleteNames)
{
    QMutexLocker locker(&m_personMutex);

    for(int i = 0; i < deleteNames.size(); i ++)
    {
        for(int j = 0; j < m_blackPersonInfos.size(); j ++)
        {
            if(m_blackPersonInfos[j].name == deleteNames[i])
            {
                if(j == 0 && m_blackPersonFeats.nPersonNum == 1)
                {
                    freeAllPersonDatabase1(&m_blackPersonFeats);
                }
                else if(j != m_blackPersonFeats.nPersonNum - 1)
                {
                    if(m_blackPersonFeats.pxPersons[j].pxFeatures)
                        free(m_blackPersonFeats.pxPersons[j].pxFeatures);

                    memcpy(&m_blackPersonFeats.pxPersons[j], &m_blackPersonFeats.pxPersons[j + 1], (m_blackPersonFeats.nPersonNum - j - 1) * sizeof(Person1));
                    m_blackPersonFeats.nPersonNum --;
                }
                else if(j == m_blackPersonFeats.nPersonNum - 1)
                {
                    if(m_blackPersonFeats.pxPersons[j].pxFeatures)
                        free(m_blackPersonFeats.pxPersons[j].pxFeatures);

                    m_blackPersonFeats.nPersonNum --;
                }

                m_blackPersonInfos.remove(j);
                j --;
            }
        }
    }
    saveBlackList(m_chanelIndex);
}

void CameraProcessEngine::loadBlackList(int chanelIndex)
{
    if(chanelIndex < 0)
        return;

    QString blackListFilePath;
    blackListFilePath.sprintf("%s/blacklist_%d.db", getBinPath().toUtf8().data(), chanelIndex);
    QFile dataFile(blackListFilePath);

    bool opened = dataFile.open(QIODevice::ReadOnly);
    if(opened == false)
    {
        setBlackInfoSize(chanelIndex, 0);
        return;
    }

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    int personCount = 0;

    dataStream >> personCount;
    freeAllPersonDatabase1(&m_blackPersonFeats);
    m_blackPersonFeats.nPersonNum = personCount;
    m_blackPersonFeats.pxPersons = (Person1*)malloc(sizeof(Person1) * personCount);

    for(int i = 0; i < personCount; i ++)
    {
        dataStream >> m_blackPersonFeats.pxPersons[i].nFeatureNum;
        m_blackPersonFeats.pxPersons[i].pxFeatures = (ARM_Feature*)malloc(sizeof(ARM_Feature) * m_blackPersonFeats.pxPersons[i].nFeatureNum);

        dataStream.readRawData((char*)m_blackPersonFeats.pxPersons[i].pxFeatures, sizeof(ARM_Feature) * m_blackPersonFeats.pxPersons[i].nFeatureNum);
    }

    for(int i = 0; i < personCount; i ++)
    {
        BlackPersonMetaInfo personInfo;
        dataStream >> personInfo.name;
        dataStream >> personInfo.gender;
        dataStream >> personInfo.birthday;
        dataStream >> personInfo.address;
        dataStream >> personInfo.description;
        dataStream >> personInfo.personType;
        dataStream >> personInfo.faceData;

        m_blackPersonInfos.append(personInfo);
    }

    dataFile.close();

    setBlackInfoSize(chanelIndex, m_blackPersonFeats.nPersonNum);
    qDebug() << "load black info" << m_chanelIndex << m_blackPersonInfos.size() << m_blackPersonFeats.nPersonNum;
}

void CameraProcessEngine::saveBlackList(int chanelIndex)
{
    if(chanelIndex < 0)
        return;

    QString blackListFilePath;
    blackListFilePath.sprintf("%s/blacklist_%d.db", getBinPath().toUtf8().data(), chanelIndex);
    QFile dataFile(blackListFilePath);

    bool opened = dataFile.open(QIODevice::WriteOnly);
    if(opened == false)
        return;

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    dataStream << m_blackPersonFeats.nPersonNum;
    for(int i = 0; i < m_blackPersonFeats.nPersonNum; i ++)
    {
        dataStream << m_blackPersonFeats.pxPersons[i].nFeatureNum;
        dataStream.writeRawData((char*)m_blackPersonFeats.pxPersons[i].pxFeatures, sizeof(ARM_Feature) * m_blackPersonFeats.pxPersons[i].nFeatureNum);
    }

    for(int i = 0; i < m_blackPersonFeats.nPersonNum; i ++)
    {
        dataStream << m_blackPersonInfos[i].name;
        dataStream << m_blackPersonInfos[i].gender;
        dataStream << m_blackPersonInfos[i].birthday;
        dataStream << m_blackPersonInfos[i].address;
        dataStream << m_blackPersonInfos[i].description;
        dataStream << m_blackPersonInfos[i].personType;
        dataStream << m_blackPersonInfos[i].faceData;
    }

    dataFile.close();

    setBlackInfoSize(chanelIndex, m_blackPersonFeats.nPersonNum);
    emit blackInfoChanged();
    qDebug() << "save black info" << m_chanelIndex << m_blackPersonInfos.size() << m_blackPersonFeats.nPersonNum;
}

void CameraProcessEngine::setFaceDetectionRunning(int running)
{
    QMutexLocker locker(&m_faceDetectionRunningMutex);

    m_faceDetectionRunning = running;
}

void CameraProcessEngine::processFaceDetection()
{
    int usbCheck = 0;
    int maxTime = 0;
    setFaceDetectionRunning(1);

    while(m_faceDetectionRunning)
    {
        if(!m_ipCamera->isRunning())
        {
            QThread::msleep(33);
            continue;
        }

        m_faceDetectionYUVData = QByteArray();
        m_ipCamera->getCurFrame(m_faceDetectionYUVData, m_faceDetectionWidth, m_faceDetectionHeight, m_faceDetectionImageTime);

        if(m_faceDetectionYUVData.size() == 0)
        {
            QThread::msleep(33);
            continue;
        }

        QByteArray yuvData = m_faceDetectionYUVData;
        int width = m_faceDetectionWidth;
        int height = m_faceDetectionHeight;
        qint64 imageTime = m_faceDetectionImageTime;

        QImage srcImage = QImage(width, height, QImage::Format_RGB888);
        convertYUV420P_toRGB888((unsigned char*)yuvData.data(), width, height, srcImage.bits());

        m_settingMutex.lock();
        int minFaceSize = DEFAULT_MIN_FACESIZE;
        if(m_cameraInfo.chkFaceSize)
            minFaceSize = m_cameraInfo.detectionFaceMinSize;

        int identifyThreshold = DEFAULT_IDENTIFY_THRESHOLD;
        if(m_cameraInfo.chkThreshold)
            identifyThreshold = m_cameraInfo.identifyThreshold;

        int blackCandidateCount = DEFAULT_BLACK_CANDIDATE_COUNT;
        if(m_cameraInfo.chkBlackCanidiateCount)
            blackCandidateCount = m_cameraInfo.blackCandidateCount;

        m_settingMutex.unlock();

        int faceCount;
        int startTime = GetTickCount();

        DetectionResult* detectionResults = FaceDetectionGray((unsigned char*)yuvData.data(), height, width, minFaceSize, &faceCount);

        if(faceCount < 0)
        {
            usbCheck = 1;
            break;
        }

        QVector<QRect> faceRects;
        for(int i = 0; i < faceCount; i ++)
        {
            faceRects.append(QRect((int)detectionResults[i].xFaceRect.rX, (int)detectionResults[i].xFaceRect.rY,
                                   (int)(28 * detectionResults[i].xFaceRect.m_rRate), (int)(28 * detectionResults[i].xFaceRect.m_rRate)));
        }

        sendFaceRects(faceRects, imageTime, srcImage);

        int endTime = GetTickCount();
        if(maxTime < endTime - startTime)
            maxTime = endTime - startTime;

        QString logStr = "Face Detection Time =  " + QString::number(endTime - startTime) + " chanel no = " + QString::number(m_chanelIndex) +
                " max = " + QString::number(maxTime) + " faceCount = " + QString::number(faceCount);
        emit logOut(logStr);

#ifdef SAVE_LOG
        FILE* fp = fopen("D:/_detection.txt", "ab");
        fprintf(fp, "%d\tchanel = %d\t%d\n", endTime - startTime, m_chanelIndex, faceCount);
        fclose(fp);
#endif
        if(faceCount)
        {
            m_modelDetectionMutex.lock();
            m_modelDetectionYUVData = yuvData;
            m_modelDetectionWidth = width;
            m_modelDetectionHeight = height;
            m_detectionResults.resize(faceCount);
            m_modelDetectionImageTime = imageTime;
            memcpy(m_detectionResults.data(), detectionResults, sizeof(DetectionResult) * faceCount);
            m_modelDetectionImage = srcImage;

            m_identifyThreshold = identifyThreshold;
            m_blackCandidateCount = blackCandidateCount;
            m_modelDetectionMutex.unlock();

            EngineFree(detectionResults);
        }

        QThread::msleep(1);
    }

    if(usbCheck == 1)
        emit usbCheckError();

    setFaceDetectionRunning(0);
}


void CameraProcessEngine::setModelDetectionRunning(int running)
{
    QMutexLocker locker(&m_modelDetectionRunningMutex);

    m_modelDetectionRunning = running;
}

void CameraProcessEngine::processModelDetection()
{
    int maxTime = 0;
    setModelDetectionRunning(1);

    QTime logTime = QTime::currentTime();
    QVector<Cluster*> clusters;
    while(m_modelDetectionRunning)
    {
        QTime oldTime = QTime::currentTime();
        QDateTime curDateTime = QDateTime::currentDateTime();
        QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults;
        m_modelDetectionMutex.lock();
        if(m_modelDetectionYUVData.size() == 0)
        {
            m_modelDetectionMutex.unlock();

            for(int i = 0; i < clusters.size(); i ++)
                clusters[i]->increaseFinish();

            QVector<LOG_ITEM> logItems;
            m_personMutex.lock();
            for(int i = 0; i < clusters.size(); i ++)
            {
                if(clusters[i]->canSave())
                {
                    LOG_ITEM logItem;
                    QVector<BLACK_RECOG_INFO> canidateInfos;
                    clusters[i]->getLogItem(&m_blackPersonInfos, curDateTime, m_chanelIndex, logItem, canidateInfos, m_blackCandidateCount, m_identifyThreshold);

                    if(logItem.featData.size())
                        logItems.append(logItem);
                    if(canidateInfos.size())
                        blackRecogResults.append(canidateInfos);

                    clusters[i]->setCanSave(false);
                }
            }
            m_personMutex.unlock();

            for(int i = 0; i < clusters.size(); i ++)
            {
                if(clusters[i]->isFinished())
                {
                    delete clusters[i];
                    clusters.remove(i);
                    i --;
                }
            }

            if(logItems.size() || blackRecogResults.size())
                saveLogs(blackRecogResults, logItems, m_chanelIndex);

            QThread::msleep(33);
            continue;
        }

        QByteArray yuvData = m_modelDetectionYUVData;
        int width = m_modelDetectionWidth;
        int height = m_modelDetectionHeight;
        QVector<DetectionResult> detectionResults = m_detectionResults;
        qint64 imageTime = m_modelDetectionImageTime;
        QImage srcImage = m_modelDetectionImage;

        m_modelDetectionYUVData.clear();
        m_detectionResults.clear();

        float identifyThreshold = m_identifyThreshold / 100.0f;
        m_modelDetectionMutex.unlock();

        int faceCount = detectionResults.size();

        int startTime = GetTickCount();
        MFResult* mfResults = MFExtractionGray((unsigned char*)yuvData.data(), height, width, detectionResults.data(), faceCount);
        int endTime = GetTickCount();

        if(faceCount)
        {
            QDateTime curDateTime = QDateTime::currentDateTime();
            QVector<MFResult> mfResulVec;
            mfResulVec.resize(faceCount);
            memcpy(mfResulVec.data(), mfResults, sizeof(MFResult) * faceCount);

#ifdef SEND_SCORE
            sendFaceScores(detectionResults, mfResults, imageTime);
#endif

            for(int i = 0; i < clusters.size(); i ++)
            {
                bool ret = clusters[i]->doClustering(srcImage, mfResulVec, detectionResults, curDateTime.toMSecsSinceEpoch());
                if(!ret)
                    clusters[i]->increaseFinish();
            }

            for(int i = 0; i < mfResulVec.size(); i ++)
            {
                Cluster* newCluster = new Cluster;
                newCluster->setFirstFeature(srcImage, mfResulVec[i], detectionResults[i], curDateTime.toMSecsSinceEpoch(), i);

                clusters.append(newCluster);
            }

            QVector<LOG_ITEM> logItems;
            m_personMutex.lock();
            for(int i = 0; i < clusters.size(); i ++)
            {
                clusters[i]->identify(&m_blackPersonFeats, &m_blackPersonInfos, identifyThreshold);

                if(clusters[i]->canSave())
                {
                    LOG_ITEM logItem;
                    QVector<BLACK_RECOG_INFO> canidateInfos;
                    clusters[i]->getLogItem(&m_blackPersonInfos, curDateTime, m_chanelIndex, logItem, canidateInfos, m_blackCandidateCount, m_identifyThreshold);

                    if(logItem.featData.size())
                        logItems.append(logItem);
                    if(canidateInfos.size())
                        blackRecogResults.append(canidateInfos);

                    clusters[i]->setCanSave(false);
                }
            }
            m_personMutex.unlock();

            for(int i = 0; i < clusters.size(); i ++)
            {
                if(clusters[i]->isFinished())
                {
                    delete clusters[i];
                    clusters.remove(i);
                    i --;
                }
            }

            if(logItems.size() || blackRecogResults.size())
            {
                saveLogs(blackRecogResults, logItems, m_chanelIndex);
            }

            EngineFree(mfResults);
        }

        if(maxTime < endTime - startTime)
            maxTime = endTime - startTime;

        QString logStr = "Modeling Time =  " + QString::number(endTime - startTime) + " chanel no = " + QString::number(m_chanelIndex) +
                " max = " + QString::number(maxTime);

        emit logOut(logStr);

#ifdef SAVE_LOG
        FILE* fp = fopen("D:/_model.txt", "ab");
        fprintf(fp, "%d\tchanel = %d\t%d\n", endTime - startTime, m_chanelIndex, faceCount);
        fclose(fp);
#endif
        QThread::msleep(1);
    }

    for(int i = 0; i < clusters.size(); i ++)
    {
        delete clusters[i];
    }

    setModelDetectionRunning(0);
}


void CameraProcessEngine::saveLogs(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults, QVector<LOG_ITEM> logItems, int chanelIndex)
{
    QTime oldTime = QTime::currentTime();
    if(logItems.size())
    {
        QByteArray sendingData;
        QBuffer sendingBuffer(&sendingData);
        sendingBuffer.open(QIODevice::WriteOnly);

        QDataStream sendingDataStream(&sendingBuffer);
        sendingDataStream << logItems.size();

    for(int i = 0; i < logItems.size(); i ++)
    {
        LOG_ITEM logItem = logItems[i];
        QByteArray capturedFaceImage = qImage2ByteArray(logItem.capturedFace);

        QDateTime curDateTime = QDateTime::currentDateTime();
        curDateTime = getDateTimeFromUID(logItem.logUID);

        QDir rootDir(getDBPath());
        rootDir.mkpath(getDBPath());

        QString dateStr = curDateTime.date().toString("yyyy-MM-dd");
        rootDir.mkdir(dateStr);

        QDir curDateDir(rootDir.absolutePath() + "/" + dateStr);
        QString chanelStr = "chanel " + QString::number(chanelIndex);
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
            logInfoStream << capturedFaceImage;
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
            logInfoStream << capturedFaceImage;
            logInfoStream << qImage2ByteArray(logItem.sceneImage);
            logInfoStream << logItem.sceneFaceRect;

            logInfoFile.close();
        }

            sendingDataStream << logItem.dateTime;

            sendingDataStream << capturedFaceImage;

            sendingDataStream << logItem.logUID;

            for(int j = 0; j < 3; j ++)
            {
                QByteArray candidateFace = qImage2ByteArray(logItem.candidateFace[j]);
                sendingDataStream << candidateFace;
                sendingDataStream << logItem.candidateDist[j];
                sendingDataStream << logItem.candidateType[j];
            }
        }

        sendingBuffer.close();

        m_logSendingMutex.lock();
        for(int i = 0; i < m_logSendingSockets.size(); i ++)
        {
            int ret = sendDataBySocket(m_logSendingSockets[i], sendingData);
            if(!ret)
            {
                closesocket(m_logSendingSockets[i]);
                m_logSendingSockets.remove(i);
                i --;
            }
        }
        m_logSendingMutex.unlock();
    }

    if(blackRecogResults.size())
    {
        QByteArray sendingData;
        QBuffer sendingBuffer(&sendingData);
        sendingBuffer.open(QIODevice::WriteOnly);

        QDataStream sendingDataStream(&sendingBuffer);
        int blackRecogResultCount = blackRecogResults.size();
        sendingDataStream << blackRecogResultCount;

        for(int i = 0; i < blackRecogResults.size(); i ++)
        {
            int canidateCount = blackRecogResults[i].size();

            for(int j = 0; j < canidateCount; j ++)
            {
                BLACK_RECOG_INFO blackRecogResult = blackRecogResults[i][j];
                QDateTime curDateTime = QDateTime::currentDateTime();
                curDateTime = getDateTimeFromUID(blackRecogResult.logUID);

                QDir rootDir(getDBPath());

                QString dateStr = curDateTime.date().toString("yyyy-MM-dd");
                rootDir.mkdir(dateStr);

                QDir curDateDir(rootDir.absolutePath() + "/" + dateStr);
                QString chanelStr = "chanel " + QString::number(chanelIndex);
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

            //////////////
            sendingDataStream << canidateCount;

            for(int j = 0; j < canidateCount; j ++)
            {
                sendingDataStream << blackRecogResults[i][j].name;
                sendingDataStream << blackRecogResults[i][j].gender;
                sendingDataStream << blackRecogResults[i][j].birthday;
                sendingDataStream << blackRecogResults[i][j].address;
                sendingDataStream << blackRecogResults[i][j].description;
                sendingDataStream << blackRecogResults[i][j].personType;
                sendingDataStream << blackRecogResults[i][j].galleryFaceData;
                sendingDataStream << blackRecogResults[i][j].probeFaceData;
                sendingDataStream << blackRecogResults[i][j].similiarity;
                sendingDataStream << blackRecogResults[i][j].logUID;
                sendingDataStream << blackRecogResults[i][j].dateTime;
            }
        }

        sendingBuffer.close();

        m_blackResultSendingMutex.lock();
        for(int i = 0; i < m_blackResultSendingSockets.size(); i ++)
        {
            int ret = sendDataBySocket(m_blackResultSendingSockets[i], sendingData);
            if(!ret)
            {
                closesocket(m_blackResultSendingSockets[i]);
                m_blackResultSendingSockets.remove(i);
                i --;
            }
        }
        m_blackResultSendingMutex.unlock();
    }
}



void CameraProcessEngine::sendFaceRects(QVector<QRect> faceRects, qint64 frameTime, QImage srcImage)
{
    QMutexLocker locker(&m_faceRectSendingMutex);

    QByteArray sendData;
    QBuffer sendingBuffer(&sendData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << faceRects;
#ifdef SEND_SCORE
    QVector<float> faceScores;
    sendingDataStream << faceScores;
#endif
    sendingDataStream << frameTime;
    sendingDataStream << QDateTime::currentDateTime().toMSecsSinceEpoch();
    sendingDataStream << qImage2ByteArray(srcImage);

    sendingBuffer.close();

    for(int i = 0; i < m_faceRectSendingSockets.size(); i ++)
    {
        //int ret = send(m_faceRectSendingSockets[i], sendData.data(), sendData.size(), 0);
        int ret = sendDataBySocket(m_faceRectSendingSockets[i], sendData);
        if(ret <= 0)
        {
            closesocket(m_faceRectSendingSockets[i]);
            m_faceRectSendingSockets.remove(i);
            i --;
            continue;
        }

        recv(m_faceRectSendingSockets[i], (char*)&ret, sizeof(int), 0);
    }
}

void CameraProcessEngine::sendFaceScores(QVector<DetectionResult> detectionResults, MFResult* mfResults, qint64 imageTime)
{
    QMutexLocker locker(&m_faceRectSendingMutex);

    QVector<QRect> faceRects;
    QVector<float> faceScores;
    for(int i = 0; i < detectionResults.size(); i ++)
    {
        faceRects.append(QRect((int)detectionResults[i].xFaceRect.rX, (int)detectionResults[i].xFaceRect.rY,
                               (int)(28 * detectionResults[i].xFaceRect.m_rRate), (int)(28 * detectionResults[i].xFaceRect.m_rRate)));
    }

    for(int i = 0; i < detectionResults.size(); i ++)
    {
        float rScore = 0;
        DistNo* distNos = Identify(&mfResults[i].xFeature, &m_blackPersonFeats, NULL, NULL, NULL, NULL);
        if(distNos)
        {
            rScore = 1 - distNos[0].rDist;
            EngineFree(distNos);

            faceScores.append(rScore);
        }
    }

    QByteArray sendData;
    QBuffer sendingBuffer(&sendData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);

    sendingDataStream << faceRects;
    sendingDataStream << faceScores;
    sendingDataStream << imageTime;
    sendingDataStream << QDateTime::currentDateTime().toMSecsSinceEpoch();

    sendingBuffer.close();

    for(int i = 0; i < m_faceRectSendingSockets.size(); i ++)
    {
        int ret = send(m_faceRectSendingSockets[i], sendData.data(), sendData.size(), 0);
        if(ret <= 0)
        {
            closesocket(m_faceRectSendingSockets[i]);
            m_faceRectSendingSockets.remove(i);
            i --;
            continue;
        }

        recv(m_faceRectSendingSockets[i], (char*)&ret, sizeof(int), 0);
    }
}

#ifdef _CREATE_THREAD_

DWORD faceDetectionProc(LPVOID param)
{
    CameraProcessEngine* cameraProcessEngine = (CameraProcessEngine*)param;
    cameraProcessEngine->processFaceDetection();
    ExitThread(0);
    return 0;
}

DWORD modelDetectionProc(LPVOID param)
{
    CameraProcessEngine* cameraProcessEngine = (CameraProcessEngine*)param;
    cameraProcessEngine->processModelDetection();
    ExitThread(0);
}

#else

void faceDetectionProc(void* param)
{
    CameraProcessEngine* cameraProcessEngine = (CameraProcessEngine*)param;
    cameraProcessEngine->processFaceDetection();
    _endthread();
}

void modelDetectionProc(void* param)
{
    CameraProcessEngine* cameraProcessEngine = (CameraProcessEngine*)param;
    cameraProcessEngine->processModelDetection();
    _endthread();
}

#endif


int convertYUVtoARGB(int Y, int U, int V, unsigned char* dstData, int index)
{
    int C = qMax(0, Y - 16);
    int D = U;
    int E = V;

    int r = (298 * C  + 409 * E + 128) >> 8;
    int g = (298 * C - 100 * D - 208 * E + 128) >> 8;
    int b = (298 * C + 516 * D + 128) >> 8;

    r = r>255? 255 : r<0 ? 0 : r;
    g = g>255? 255 : g<0 ? 0 : g;
    b = b>255? 255 : b<0 ? 0 : b;

    dstData[index * 3] = r;
    dstData[index * 3 + 1] = g;
    dstData[index * 3 + 2] = b;

    return 0;
}

void convertYUV420P_toRGB888(unsigned char* data, int width, int height, unsigned char* dstData)
{
    int size = width*height;
    int offset = size;
    int u, v, y1, y2, y3, y4;

    for(int i = 0, k = offset; i < size; i += 2, k ++)
    {
        y1 = data[i];
        y2 = data[i + 1];
        y3 = data[width + i];
        y4 = data[width + i + 1];

        v = data[k + width * height / 4];
        u = data[k];
        v = v-128;
        u = u-128;

        convertYUVtoARGB(y1, u, v, dstData, i);
        convertYUVtoARGB(y2, u, v, dstData, i + 1);
        convertYUVtoARGB(y3, u, v, dstData, width + i);
        convertYUVtoARGB(y4, u, v, dstData, width + i + 1);

        if (i!=0 && (i+2)%width==0)
            i += width;
    }
}

