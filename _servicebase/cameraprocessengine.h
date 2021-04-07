#ifndef CAMERAPROCESSENGINE_H
#define CAMERAPROCESSENGINE_H

#include "servicebase.h"
#include <QThread>

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QImage>

#include <winsock.h>
//#include <winsock2.h>

#include "base.h"
#include "ipcamera.h"

class CameraProcessEngine : public QObject
{
    Q_OBJECT
public:
    explicit CameraProcessEngine(QObject *parent = 0);
    ~CameraProcessEngine();

    void    startProcess(int chanelIndex, IpCameraInfo cameraInfos);
    void    stopProcess();

    int     status();

    IpCameraInfo    cameraInfos();
    void    setIpCameraInfo(IpCameraInfo cameraInfos);
    void    setSurveillanceSetting(IpCameraInfo cameraInfos);

    void    addFaceRectSendingSocket(SOCKET socket);
    void    addLogSendingSocket(SOCKET socket);
    void    addBlackSendingSocket(SOCKET socket);

    int     chanelIndex();

    QVector<BlackPersonMetaInfo> blackPersonMeataInfo();
    PersonDatabase1 blackPersonDatabase();

    void    setFaceDetectionRunning(int running);
    void    setModelDetectionRunning(int running);

    void    processFaceDetection();
    void    processModelDetection();
signals:
    void    savedLogInfo(QDate);
    void    resultChanged(QVector<QRect> faceResults, qint64);
    void    logChanged(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults, QVector<LOG_ITEM> logItems, int chanelIndex);
    void    logOut(QString logOutStr);

    void    blackInfoChanged();
    void    usbCheckError();

public slots:
    void    slotAddOneBlackPerson(Person1 personFeat,BlackPersonMetaInfo personInfo);
    void    slotAddBlackPersonData(QByteArray blackPersonData);
    void    slotEditOneBlackPerson(QString oldName, Person1 personFeat, BlackPersonMetaInfo personInfo);
    void    slotDeleteBlackInfos(QStringList deleteNames);

private:
    void    loadBlackList(int chanelIndex);
    void    saveBlackList(int chanelIndex);

    void    saveLogs(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults, QVector<LOG_ITEM> logItems, int chanelIndex);
    void    sendFaceRects(QVector<QRect> faceRects, qint64 frameTime, QImage srcImage);
    void    sendFaceScores(QVector<DetectionResult> detectionResults, MFResult* mfResults, qint64 imageTime);

private:
    HANDLE				m_faceDetectionThread;
    HANDLE				m_modelDetectionThread;

    QByteArray          m_faceDetectionYUVData;
    qint64              m_faceDetectionImageTime;
    int                 m_faceDetectionWidth;
    int                 m_faceDetectionHeight;

    QVector<DetectionResult> m_detectionResults;
    QByteArray          m_modelDetectionYUVData;
    int                 m_modelDetectionWidth;
    int                 m_modelDetectionHeight;    
    qint64              m_modelDetectionImageTime;
    QImage              m_modelDetectionImage;

    int                 m_faceDetectionRunning;
    int                 m_modelDetectionRunning;
    int                 m_cameraLoginRunning;
    QMutex              m_faceDetectionRunningMutex;
    QMutex              m_modelDetectionRunningMutex;

    QMutex              m_faceDetectionMutex;
    QMutex              m_modelDetectionMutex;
    QMutex              m_personMutex;
    QMutex              m_settingMutex;

    IpCameraInfo    m_cameraInfo;
    int             m_chanelIndex;
    int             m_identifyThreshold;
    int             m_blackCandidateCount;

    PersonDatabase1    m_blackPersonFeats;
    QVector<BlackPersonMetaInfo>    m_blackPersonInfos;


    IpCamera*       m_ipCamera;

    QVector<SOCKET> m_faceRectSendingSockets;
    QMutex          m_faceRectSendingMutex;
    QVector<SOCKET> m_logSendingSockets;
    QMutex          m_logSendingMutex;
    QVector<SOCKET> m_blackResultSendingSockets;
    QMutex          m_blackResultSendingMutex;
};

#endif // CAMERAPROCESSENGINE_H
