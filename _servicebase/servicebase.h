#ifndef SERVICEBASE_H
#define SERVICEBASE_H

#include <QtWidgets>

#include "frengine.h"


#define SERVICE_NAME                 L"VMSService - Video"
#define SERVICE_DESC                 L"Easen FaceDetection Service - Video"
#define SERVICE_ARG_INSTALL          L"-install"
#define SERVICE_ARG_UNINSTALL        L"-uninstall"
#define SERVICE_ARG_SPAWN_PROVIDER   L"-spawnprovider"
#define SERVICE_LOGFILE "D:/service.txt"

#define DEFAULT_MIN_FACESIZE 8
#define DEFAULT_IDENTIFY_THRESHOLD 90
#define DEFAULT_BLACK_CANDIDATE_COUNT 3


typedef struct _tagPersonMetaInfo
{
    QString name;
    int     gender;
    QDate   birthday;
    QString address;
    QString description;
    int     personType;
    QVector<QByteArray> faceData;
}BlackPersonMetaInfo;

typedef struct _LOG_ITEM
{
    QString         logUID;
    QDateTime       dateTime;
    QByteArray      featData;
    QImage          capturedFace;
    QImage          sceneImage;
    QImage          candidateFace[3];
    float           candidateDist[3];
    int             candidateType[3];
    QRect           sceneFaceRect;
    int             isFinished;
}LOG_ITEM;

typedef struct _tagSCENE_LOG_ITEM
{
    QVector<LOG_ITEM> logItems;
    QImage            sceneImage;
    QString           sceneUID;
}SCENE_LOG_ITEM;

typedef struct _tagBlackRecogInfo
{
    QString name;
    int     gender;
    QDate   birthday;
    QString address;
    QString description;
    int     personType;
    QByteArray  galleryFaceData;
    QByteArray  probeFaceData;
    float       similiarity;

    QByteArray      featData;
    QImage          sceneImage;
    QRect           sceneFaceRect;

    QString         logUID;
    QDateTime       dateTime;
}BLACK_RECOG_INFO;

typedef struct _tagSENDING_FRAME_RESULT
{
    QByteArray  featData;
    QRect       faceRect;
}SENDING_FRAME_RESULT;


typedef struct _tagIpCamreaInfo
{
    QString ipAddress;
    int     portNum;
    QString videoSource;
    QString streamUri;

    int     chkFaceSize;
    int     detectionFaceMinSize;
    int     chkThreshold;
    int     identifyThreshold;

    int     chkBlackCanidiateCount;
    int     blackInfoSize;
    int     blackCandidateCount;
}IpCameraInfo;

typedef struct _tagLOG_ITEM_INFO
{
    QString     thumbName;
    int         thumbSize;
    QString     sceneName;
    int         sceneSize;
    QString     featureName;
    int         featureSize;
    QString     blackName;
    int         blackSize;

}LOG_ITEM_INFO;

#define ORG_NAME "Easen"
#define APP_NAME "FaceDetectionService_video"

#define CAM_STATUS_STOP 0
#define CAM_STATUS_RUNNING 1

QRect getFrameRect(QRect boundingRect, QSize imageSize);
QImage SImg2QImage(SImg* img);
SImg qImage2SImg(QImage colorImage);
void freeSImg(SImg* img);
SImg createSImg(int width, int height);
void copySImg(SImg* dstImage, SImg* srcImage);

void getScaledWorkImage(QImage srcFrame, QImage& dstImage, double& scale);
//void getLogItemAndSave(QVector<LOG_ITEM>& logItems, TrackSectionResult* trackSectionResult, int sectionCount, QDateTime curDateTime, QString cameraUIDStr);

QString getUserName();
QString getPassword();
QString getServerInfoStr();
int     getChanelCount();
QVector<IpCameraInfo>   getIpCameraInfos();
IpCameraInfo            getIpCameraInfo(int index);
void setIpCameraInfo(int index, IpCameraInfo ipCameraInfo);
void setSurveillanceSetting(int index, IpCameraInfo ipCameraInfo);
void setBlackInfoSize(int index, int blackInfoSize);

int     getHDSerial();

void    freePerson1(Person1 person1);
void    freePersonDatabase1(PersonDatabase1 person1);
void    freeAllPersonDatabase1(PersonDatabase1* person1);

QByteArray qImage2ByteArray(QImage image);
QImage qByteArray2Image(QByteArray imageData);
QImage cropFaceImage(QImage frameImage, QRect faceRect);
QImage cropSceneImage(QImage frameImage, QRect faceRect, QRect& sceneFaceRect);

QByteArray getSceneData(QString logUID);
QString getSceneUID(QString logUID);
QString getLogUID(QString sceneUID, int index);

QString getDBPath();
void    setDBPath(QString);
QString getBinPath();
QDateTime getDateTimeFromUID(QString uidStr);

#endif // BASE_H
