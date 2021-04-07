#ifndef CLUSTER_H
#define CLUSTER_H

#include "servicebase.h"
#include "frengine.h"

class Cluster : public QObject
{
    Q_OBJECT
public:
    explicit Cluster(QObject *parent = 0);

    bool    doClustering(QImage srcImage, QVector<MFResult>& mfResults, QVector<DetectionResult>& detectionResults, qint64 currentTime);
    void    setFirstFeature(QImage firstImage, MFResult mfResult, DetectionResult detectionResult, qint64 featureTime, int clusterIndex);
    void    addFeature(QByteArray featData, QRect faceRect, qint64 featureTime);

    QDateTime clusterTime();
    QImage    clusterImage();
    QRect     clusterRect();
    QByteArray clusterFeat();

    QImage    curImage();
    QRect    curRect();
    QByteArray  curFeat();

    bool    isValide();
    QVector<QByteArray> feats();

    bool    hasName();
    bool    canSave();
    void    setCanSave(bool);
    void    identify(PersonDatabase1* personDatabase, QVector<BlackPersonMetaInfo>* blackMetaInfos, float identifyThreshold);
    void    getLogItem(QVector<BlackPersonMetaInfo>* blackMetaInfo, QDateTime logTime, int chanelIndex, LOG_ITEM& logItem,
                       QVector<BLACK_RECOG_INFO>& candidateRecogInfo, int blackCandiateCount, float identifyThreshold);

    void    setFirstFeature(QString rootPath, QString featureName);
    void    addFeature(QString featureName);

    bool    canMerge(Cluster*);
    void    mergeCluster(Cluster* cluster);
    float   getScore(Cluster*);

    void    increaseFinish();

    bool    isFinished();
    void    setFinished();

    void    save(int index);

    qint64  lastFileTime();
    qint64  firstFileTime();
    QByteArray lastFileFeat();
    QByteArray firstFileFeat();
    QRect   lastFileRect();
    QRect   firstFileRect();

    QStringList featurePath();
    QVector<QRect> galleryRect();
    QVector<QByteArray> galleryFeat();


signals:

public slots:

private:
    float   getScore(QByteArray galleryFeat, QByteArray probeFeat);
    int     euclideDistance(QRect rect1, QRect rect2);

private:
    QStringList m_featurePath;
    QString     m_rootPath;

    QVector<QRect>      m_galleryRect;
    QVector<QByteArray>  m_galleryFeat;

    int         m_finished;

    QVector<QByteArray>  m_feats;
    QVector<QRect>       m_faceRects;
    qint64      m_lastTime;
    qint64      m_firstTime;
    qint64      m_saveTime;

    QImage      m_goodImage;
    QImage      m_curImage;
    QRect       m_goodRect;

    float       m_pan;
    float       m_modelConfidence;
    QByteArray  m_goodFeature;

    int         m_clusterIndex;
    bool        m_canSave;
    QString     m_oldName;
    int         m_oldIdentifyIndex;

    QVector<DistNo> m_identifyResult;
};

#endif // CLUSTER_H
