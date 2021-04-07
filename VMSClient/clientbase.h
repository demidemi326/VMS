#ifndef CLIENTBASE_H
#define CLIENTBASE_H

#include "enginestruct.h"

#define CAMERA_VIEW_MAX_COL 4
#define CAMERA_VIEW_MAX_ROW 4

#define CAMERA_VIEW_1_1 0
#define CAMERA_VIEW_2_2 1
#define CAMERA_VIEW_3_3 2
#define CAMERA_VIEW_4_4 3

#define DEFAULT_MIN_FACESIZE 8
#define DEFAULT_IDENTIFY_THRESHOLD 90
#define DEFAULT_BLACK_CANDIDATE_COUNT 3

#define N_MAX_BLACK_RECOG_RESULT 200
#define N_SEARCH_LOG_PAGE 100
#define N_SEARCH_BLACK_PAGE 100

#define AREA_INDEX_ROLE Qt::UserRole + 2
#define SERVER_INDEX_ROLE Qt::UserRole + 3
#define CHANEL_INDEX_ROLE Qt::UserRole + 4

#define ROW_INDEX_ROLE Qt::UserRole + 5
#define LOG_UID_ROLE Qt::UserRole + 6

#define GROUP_NAME_ROLE Qt::UserRole + 7
#define GROUP_DATETIME_ROLE Qt::UserRole + 8

#define ALRM_ID_ROLE Qt::UserRole + 9
#define ALRM_FLAG_ROLE Qt::UserRole + 10

typedef struct _tagIpCamreaInfo
{
    QString ipAddress;
    int     portNum;
    QString videoSource;
    QString streamuri;

    int     chkFaceSize;
    int     detectionFaceMinSize;
    int     chkThreshold;
    int     identifyThreshold;
    int     chkBlackCandidateCount;
    int     blackCandidateCount;

    int     blackInfoSize;
}IpCameraInfo;

typedef struct _tagServerInfo
{
    QString serverName;
    QString ipAddress;
    int     port;
    QString userName;
    QString password;

    QString serverTypeStr;
    int     serverUID;
    QVector<IpCameraInfo>   ipCameraInfos;
}ServerInfo;

typedef struct _tagMonitoringAreaInfo
{
    QString         areaName;
    QVector<int>    serverIndexs;
    QVector<int>    chanelIndexs;
}MonitoringAreaInfo;

typedef struct _tagLOG_RESULT
{
    int             serverUID;
    int             chanelIndex;

    QString         areaName;
    QString         logUID;
    QDateTime       dateTime;
    QByteArray      faceImage;
    QByteArray      candidateFace[3];
    float           candidateDist[3];
    int             candidateType[3];
}LOG_RESULT;

typedef struct _tagFRAME_RESULT
{
    QByteArray  featData;
    QRect       faceRect;
    QImage      faceImage;
}FRAME_RESULT;

typedef struct _tagBLACK_PERSON
{
    QVector<QByteArray> featData;
    QVector<QByteArray> faceData;
    QString             name;
    int                 gender;
    QDate               birthDay;
    QString             address;
    QString             description;
    int     personType;
}BLACK_PERSON;

typedef struct _tagBLACK_RECOG_RESULT
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

    QString     areaName;
    QString     logUID;
    QDateTime   dateTime;
    int         serverUID;
    int         chanelIndex;
}BLACK_RECOG_RESULT;


typedef enum _E_GENDER
{
    EG_MALE,
    EG_FEMALE
}E_GENDER;


#define CACHE_LOG_NAME "./cachelog_video.bin"
#define CACHE_LOG_SIZE 8000

#define CACHE_BLACK_NAME "./cacheblack_video.bin"
#define CACHE_BLACK_SIZE 16000

#define ORG_NAME "Easen"
#define APP_NAME "VMSClient_video"

QRect getFrameRect(QRect boundingRect, QSize imageSize);
SImg qImage2SImg(QImage);
void freeSImg(SImg);

QByteArray qImage2ByteArray(QImage image);
QImage  qByteArray2Image(QByteArray imageData);

QImage cropFaceImage(QImage frameImage, QRect faceRect);

class ServerInfoSocket;
int getServerIndexFromUID(QVector<ServerInfoSocket*> serverInfoSockets, int serverUID);
int getAreaIndexFromServerIndex(QVector<MonitoringAreaInfo> areaInfo, int serverIndex, int chanelIndex);

QString getServerNameFromUID(QVector<ServerInfoSocket*> serverInfoSockets, int serverUID);
QString getAreaNameFromUID(QVector<MonitoringAreaInfo> areaInfos, QVector<ServerInfoSocket*> serverInfoSockets, int serverUID, int chanelIndex);

QString readPass();
void writePass(QString pass);

#endif // BASE_H
