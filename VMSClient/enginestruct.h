#ifndef ENGINESTRUCT_H
#define ENGINESTRUCT_H

#include <QtWidgets>

typedef struct _tagSRECT
{
    int     left;
    int     top;
    int     width;
    int     height;
}SRECT;

typedef struct _tagSPOINT
{
    float     x;
    float     y;
}SPOINT;

typedef struct _tagSystemParam
{
 int nMinPercentage;
 int nMaxPercentage;
 float rFrameDistTH;
 float rTrackDistTH;
 int nTrackDelayNum;
 int nSectionLen;
 float rNeighbourOffset;
 int nShotThresold;
}SystemParam;

typedef struct _LOG_ITEM
{
    QDateTime       dateTime;
    QString         name;
    int             gender;
    int             age;
    QImage          capturedFace;
    QImage          rankFace1;
    QImage          rankFace2;
    QImage          rankFace3;
    float           rank1;
    float           rank2;
    float           rank3;
    int             mode;
}LOG_ITEM;

typedef struct _tagSImg
{
 int nH;
 int nW;
 unsigned char* pbImage;
}SImg;

typedef struct _tagFrameResult
{
    SRECT xFaceRegion;
    char szName[256];
    int nAge;
    int nSex;
    int nHappy;
    int nSad;
    int nAngry;
    int nSurprise;
    float rLEX, rLEY, rREX, rREY;
    int nUpdateFlag;
}FrameResult;

typedef struct _tagTrackSectionResult
{
    SImg* pmCaptured;
    SImg* pmCandidate[3];
    float prDist[3];
    char szName[256];
    int nAge;
    int nSex;
}TrackSectionResult;


typedef struct _tagDistNo
{
 float rDist;
 int nNo;
}DistNo;

#endif // ENGINESTRUCT_H
