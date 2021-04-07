#include "cluster.h"
#include "frengine.h"
#include "servicebase.h"

#include <QtWidgets>

#define SCORE_THRESHOLD 0.8
#define TIME_THRESHOLD 1000
#define TIME_SWAP 500

#define PI 3.141592

Cluster::Cluster(QObject *parent) :
    QObject(parent)
{
    m_finished = 0;
    m_pan = 90;
    m_modelConfidence = 0;
    m_firstTime = m_lastTime = m_saveTime = 0;
    m_canSave = false;
    m_oldIdentifyIndex = -1;
}

bool Cluster::doClustering(QImage srcImage, QVector<MFResult>& mfResults, QVector<DetectionResult>& detectionResults, qint64 currentTime)
{
    if(mfResults.size() == 0 || mfResults.size() != detectionResults.size())
        return false;

    if(currentTime - m_lastTime > TIME_THRESHOLD)
    {
        m_finished = 30;
        setCanSave(true);
        return false;
    }

    float maxScore = 0;
    int nearIndex = -1;
    for(int i = 0; i < mfResults.size(); i ++)
    {
        QByteArray probeFeat((char*)&mfResults[i].xFeature, sizeof(ARM_Feature));
        QRect probeRect = QRect((int)detectionResults[i].xFaceRect.rX, (int)detectionResults[i].xFaceRect.rY,
                               (int)(28 * detectionResults[i].xFaceRect.m_rRate), (int)(28 * detectionResults[i].xFaceRect.m_rRate));

        if(!m_faceRects[m_faceRects.size() - 1].intersects(probeRect))
            continue;

        QRect intersect = m_faceRects[m_faceRects.size() - 1].intersected(probeRect);

        float rScore = getScore(m_feats[m_feats.size() - 1], probeFeat);
        float intersectScore = (intersect.width()* intersect.height()) / (float)(m_faceRects[m_faceRects.size() - 1].width() * m_faceRects[m_faceRects.size() - 1].height() + probeRect.width() * probeRect.height());
        if(rScore > maxScore && intersectScore > 0.2)
        {
            maxScore = rScore;
            nearIndex = i;
        }
    }

    if(nearIndex < 0)
        return false;

    if(maxScore < 0.5)
        return false;

    float rPan = fabs(detectionResults[nearIndex].rPan);
    float rModelConfidence = mfResults[nearIndex].rModelConfidence;

    QByteArray probeFeat((char*)&mfResults[nearIndex].xFeature, sizeof(ARM_Feature));
    QRect probeRect = QRect((int)detectionResults[nearIndex].xFaceRect.rX, (int)detectionResults[nearIndex].xFaceRect.rY,
                           (int)(28 * detectionResults[nearIndex].xFaceRect.m_rRate), (int)(28 * detectionResults[nearIndex].xFaceRect.m_rRate));

    int borderW = srcImage.height() / 15;
    if((m_pan > rPan && m_pan > 5) || (m_pan <= 5 && rPan <= 5 && m_goodRect.width() < probeRect.width() &&
                                       probeRect.left() > borderW && probeRect.top() > borderW &&
                                       probeRect.right() < srcImage.width() - borderW && probeRect.bottom() < srcImage.height() - borderW))
    {
        if(m_modelConfidence < rModelConfidence || rModelConfidence > 0.7)
        {
            m_pan = rPan;
            m_modelConfidence = rModelConfidence;

            m_goodRect = probeRect;
            m_goodImage = srcImage;
            m_goodFeature = probeFeat;
        }
    }

    m_curImage = srcImage;
    addFeature(probeFeat, probeRect, currentTime);

    mfResults.remove(nearIndex);
    detectionResults.remove(nearIndex);

    m_finished = 0;

    return true;
}

void Cluster::setFirstFeature(QImage firstImage, MFResult mfResult, DetectionResult detectionResult, qint64 featureTime, int clusterIndex)
{
    QByteArray featData((char*)&mfResult.xFeature, sizeof(ARM_Feature));
    QRect faceRect = QRect((int)detectionResult.xFaceRect.rX, (int)detectionResult.xFaceRect.rY,
                           (int)(28 * detectionResult.xFaceRect.m_rRate), (int)(28 * detectionResult.xFaceRect.m_rRate));

    m_curImage = firstImage;
    m_goodImage = firstImage;
    m_goodRect = faceRect;
    m_clusterIndex = clusterIndex;

    m_feats.clear();
    m_faceRects.clear();

    m_feats.append(featData);
    m_faceRects.append(faceRect);
    m_lastTime = m_firstTime = featureTime;
}

void Cluster::addFeature(QByteArray featData, QRect faceRect, qint64 featureTime)
{
    if(featureTime - m_saveTime > TIME_SWAP)
    {
        m_saveTime = featureTime;
        setCanSave(true);
    }

    m_feats.append(featData);
    m_faceRects.append(faceRect);
    m_lastTime = featureTime;

    if(m_lastTime - m_firstTime > 60 * 1000)
    {
        m_finished = 30;
        setCanSave(true);
    }
}

QDateTime Cluster::clusterTime()
{
    return QDateTime::fromMSecsSinceEpoch(m_lastTime);
}

QImage Cluster::clusterImage()
{
    return m_goodImage;
}

QRect Cluster::clusterRect()
{
    return m_goodRect;
}

QByteArray Cluster::clusterFeat()
{
    return m_goodFeature;
}

QImage Cluster::curImage()
{
    return m_curImage;
}

QRect Cluster::curRect()
{
    return m_faceRects[m_faceRects.size() - 1];
}

QByteArray Cluster::curFeat()
{
    return m_feats[m_feats.size() - 1];
}

bool Cluster::isValide()
{
    return m_feats.size() > 5;
}

QVector<QByteArray> Cluster::feats()
{
    return m_feats;
}

bool Cluster::canSave()
{
    return m_canSave;
}

bool Cluster::hasName()
{
    return !m_oldName.isEmpty();
}

void Cluster::setCanSave(bool canSave)
{
    m_canSave = canSave;
}

void Cluster::identify(PersonDatabase1* personDatabase, QVector<BlackPersonMetaInfo>* blackMetaInfos, float identifyThreshold)
{
    m_identifyResult.clear();
    if(personDatabase == NULL || blackMetaInfos == NULL || blackMetaInfos->size() != personDatabase->nPersonNum)
        return;

    if(personDatabase->nPersonNum != 0)
    {
        Person1 person1 = { 0 };
        person1.nFeatureNum = m_feats.size();
        person1.pxFeatures = (ARM_Feature*)malloc(person1.nFeatureNum * sizeof(ARM_Feature));
        for(int i = 0; i < m_feats.size(); i ++)
            memcpy(&person1.pxFeatures[i], (ARM_Feature*)m_feats[i].data(), sizeof(ARM_Feature));


        DistNo* distNos = IdentifySet(&person1, personDatabase, NULL, NULL, NULL, NULL);
        if(distNos != NULL)
        {
            for(int i = 0; i < personDatabase->nPersonNum; i ++)
                m_identifyResult.append(distNos[i]);

            if(distNos)
                EngineFree(distNos);

            free(person1.pxFeatures);

            if((1 - m_identifyResult[0].rDist) > identifyThreshold)
            {
                if(m_oldName != blackMetaInfos->at(m_identifyResult[0].nNo).name)
                {
                    m_oldName = blackMetaInfos->at(m_identifyResult[0].nNo).name;

                    m_saveTime = m_lastTime;
                    setCanSave(true);
                }
            }
        }
    }
}

void Cluster::getLogItem(QVector<BlackPersonMetaInfo>* blackMetaInfo, QDateTime logTime, int chanelIndex, LOG_ITEM& logItem,
                         QVector<BLACK_RECOG_INFO>& candidateRecogInfo, int blackCandiateCount, float identifyThreshold)
{
    if(!canSave())
        return;

    if(blackMetaInfo == NULL || blackMetaInfo->size() != m_identifyResult.size())
        return;

    logItem.dateTime = logTime;
    logItem.logUID = getLogUID(QString::number(m_firstTime) + "_" + QString::number(chanelIndex), m_clusterIndex);

    if(isFinished())
    {
        logItem.capturedFace = cropFaceImage(m_goodImage, m_goodRect);
        logItem.sceneImage = cropSceneImage(m_goodImage, m_goodRect, logItem.sceneFaceRect);
        logItem.featData = m_goodFeature;
        logItem.isFinished = 1;
    }
    else
    {
        logItem.capturedFace = cropFaceImage(m_curImage, m_faceRects[m_faceRects.size() - 1]);
        logItem.sceneImage = cropSceneImage(m_curImage, m_faceRects[m_faceRects.size() - 1], logItem.sceneFaceRect);
        logItem.featData = m_feats[m_feats.size() - 1];
        logItem.isFinished = 0;
    }

    int candidateCount = qMin(3, m_identifyResult.size());
    for(int i = 0; i < candidateCount; i ++)
    {
        int index = m_identifyResult[i].nNo;
        logItem.candidateFace[i] = QImage::fromData(blackMetaInfo->at(index).faceData[0], "JPG");
        logItem.candidateDist[i] = m_identifyResult[i].rDist;
        logItem.candidateType[i] = blackMetaInfo->at(index).personType;
    }


    if(!m_oldName.isEmpty())
    {
        int candiateCount = qMin(blackCandiateCount, m_identifyResult.size());
        for(int i = 0; i < candiateCount; i ++)
        {
            BLACK_RECOG_INFO blackRecogInfo;
            if((1 - m_identifyResult[i].rDist) * 100 < identifyThreshold)
                continue;

            int index = m_identifyResult[i].nNo;
            blackRecogInfo.name = blackMetaInfo->at(index).name;
            blackRecogInfo.gender = blackMetaInfo->at(index).gender;
            blackRecogInfo.birthday = blackMetaInfo->at(index).birthday;
            blackRecogInfo.address = blackMetaInfo->at(index).address;
            blackRecogInfo.description = blackMetaInfo->at(index).description;
            blackRecogInfo.personType = blackMetaInfo->at(index).personType;
            blackRecogInfo.galleryFaceData = blackMetaInfo->at(index).faceData[0];
            blackRecogInfo.probeFaceData = qImage2ByteArray(logItem.capturedFace);
            blackRecogInfo.similiarity = 1 - m_identifyResult[i].rDist;
            blackRecogInfo.logUID = getLogUID(QString::number(m_lastTime + i) + "_" + QString::number(chanelIndex), m_clusterIndex);
            blackRecogInfo.dateTime = QDateTime::fromMSecsSinceEpoch(m_lastTime);
            blackRecogInfo.sceneImage = logItem.sceneImage;
            blackRecogInfo.sceneFaceRect = logItem.sceneFaceRect;
            blackRecogInfo.featData = logItem.featData;

            candidateRecogInfo.append(blackRecogInfo);
        }
    }
}


void Cluster::setFirstFeature(QString rootPath, QString featureName)
{
    m_rootPath = rootPath;

    m_featurePath.clear();
    m_galleryFeat.clear();
    m_featurePath.append(featureName);

    QByteArray galleryFeat;
    QRect galleryRect;
    QFile galleryFile(m_rootPath + "/_feature/" + featureName);
    if(!galleryFile.open(QIODevice::ReadOnly))
        return;

    QDataStream galleryStream(&galleryFile);
    galleryStream >> galleryFeat;
    galleryStream >> galleryRect;
    galleryFile.close();

    m_galleryFeat << galleryFeat;
    m_galleryRect << galleryRect;
}

void Cluster::addFeature(QString featureName)
{
    m_featurePath.append(featureName);

    QByteArray galleryFeat;
    QRect galleryRect;
    QFile galleryFile(m_rootPath + "/_feature/" + featureName);
    if(!galleryFile.open(QIODevice::ReadOnly))
        return;

    QDataStream galleryStream(&galleryFile);
    galleryStream >> galleryFeat;
    galleryStream >> galleryRect;
    galleryFile.close();

    m_galleryFeat << galleryFeat;
    m_galleryRect << galleryRect;
}

void Cluster::increaseFinish()
{
    m_finished ++;
    if(isFinished())
        setCanSave(true);
}

bool Cluster::isFinished()
{
    return m_finished > 7;
}

void Cluster::setFinished()
{
    m_finished = 30;
    setCanSave(true);
}

void Cluster::save(int index)
{
    if(m_featurePath.size() < 5)
        return;

    QDir rootDir(m_rootPath);
    rootDir.mkdir("result");

    QStringList splitList = m_featurePath[0].split(".bin");
    if(splitList.size() != 2)
        return;

    QString prefixName = splitList[0];
//    QDir resultDir(m_rootPath + "/result");
//    resultDir.mkdir(prefixName);

    for(int i = 0; i < m_featurePath.size(); i ++)
    {
        QStringList splitList = m_featurePath[i].split(".bin");
        if(splitList.size() != 2)
            break;

        QString baseName = splitList[0];

        QString firstName = m_rootPath + "/_thumbLog/" + baseName + ".jpg";
        QString secondName = m_rootPath + "/result/" + QString::number(index) + "_" + baseName + ".jpg";

        QFile::copy(firstName, secondName);

//        firstName = m_rootPath + "/_feature/" + baseName + ".bin";
//        secondName = m_rootPath + "/result/" + prefixName + "/" + baseName + ".bin";

//        QFile::copy(firstName, secondName);
    }
}

float Cluster::getScore(QByteArray galleryFeat, QByteArray probeFeat)
{
//    ARM_Feature galleryFeature;
//    memcpy(&galleryFeature, galleryFeat.data(), galleryFeat.size());

//    PersonDatabase1 personDatabase;
//    personDatabase.nPersonNum = 1;
//    personDatabase.pxPersons = (Person1*)malloc(sizeof(Person1));
//    personDatabase.pxPersons->nFeatureNum = 1;
//    personDatabase.pxPersons->pxFeatures = (ARM_Feature*)probeFeat.data();

//    DistNo* distNos = (DistNo*)Identify(&galleryFeature, &personDatabase, NULL, NULL, NULL, NULL);

//    float rDist = distNos[0].rDist;
//    EngineFree(distNos);
//    free(personDatabase.pxPersons);
    DistNo* distNos = IdentifyCPU(probeFeat.data(), galleryFeat.data(), 1);
    float rDist = distNos[0].rDist;
    EngineFree(distNos);

    return 1 - rDist;
}

int Cluster::euclideDistance(QRect rect1, QRect rect2)
{
    return /*sqrt*/(((rect1.left() + rect1.right()) / 2 - (rect2.left() + rect2.right()) / 2) * ((rect1.left() + rect1.right()) / 2 - (rect2.left() + rect2.right()) / 2) +
                ((rect1.top() + rect1.bottom()) / 2 - (rect2.top() + rect2.bottom()) / 2) * ((rect1.top() + rect1.bottom()) / 2 - (rect2.top() + rect2.bottom()) / 2));
}

bool Cluster::canMerge(Cluster* cluster)
{
    float rScore = getScore(cluster);
    if(rScore > 0.93)
        return true;

    return false;
}

void Cluster::mergeCluster(Cluster* cluster)
{
    m_featurePath << cluster->featurePath();
    m_galleryRect << cluster->galleryRect();
    m_galleryFeat << cluster->galleryFeat();

    m_finished = false;
}

bool lessThanScore( const float & e1, const float & e2 )
{
    if(e1 < e2)
        return false;
    else
        return true;
}

float Cluster::getScore(Cluster* cluster)
{
    QByteArray galleryFeat = m_galleryFeat[m_galleryFeat.size() - 1];
    QByteArray probeFeat = cluster->galleryFeat().at(0);
    return getScore(galleryFeat, probeFeat);

//    QVector<QByteArray> probeFeat = cluster->galleryFeat();

//    QVector<float> distMatrix;

//    double rSumScore = 0;
//    for(int i = 0; i < m_galleryFeat.size(); i ++)
//    {
//        for(int j = 0; j < probeFeat.size(); j ++)
//        {
//            float rScore = getScore(m_galleryFeat[i], probeFeat[j]);
//            rSumScore += rScore;
//            distMatrix.append(rScore);
//        }
//    }

//    qSort(distMatrix.begin(), distMatrix.end(), lessThanScore);

//    if(distMatrix.size() == 0)
//        return 0;

////    return rSumScore / distMatrix.size();

//    if(distMatrix.size() == 1)
//        return distMatrix[0];

//    return (distMatrix[0] + distMatrix[1]) / 2;
}

qint64 Cluster::lastFileTime()
{
    if(m_featurePath.size() == 0)
        return 0;

    QStringList splitList = m_featurePath[m_featurePath.size() - 1].split("_");
    if(splitList.size() != 3)
        return 0;

    return splitList[0].toLongLong();
}

qint64 Cluster::firstFileTime()
{
    if(m_featurePath.size() == 0)
        return 0;

    QStringList splitList = m_featurePath[0].split("_");
    if(splitList.size() != 3)
        return 0;

    return splitList[0].toLongLong();
}

QByteArray Cluster::lastFileFeat()
{
    if(m_galleryFeat.size() == 0)
        return QByteArray();

    return m_galleryFeat[m_galleryFeat.size() - 1];
}

QByteArray Cluster::firstFileFeat()
{
    if(m_galleryFeat.size() == 0)
        return QByteArray();

    return m_galleryFeat[0];
}

QRect Cluster::lastFileRect()
{
    if(m_galleryRect.size() == 0)
        return QRect();

    return m_galleryRect[m_galleryRect.size() - 1];
}

QRect Cluster::firstFileRect()
{
    if(m_galleryRect.size() == 0)
        return QRect();

    return m_galleryRect[0];
}


QStringList Cluster::featurePath()
{
    return m_featurePath;
}

QVector<QRect> Cluster::galleryRect()
{
    return m_galleryRect;
}

QVector<QByteArray> Cluster::galleryFeat()
{
    return m_galleryFeat;
}
