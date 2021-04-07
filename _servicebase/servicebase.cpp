

#include "servicebase.h"
#include "socketbase.h"
#include "base.h"

#include <QtWidgets>


QRect getFrameRect(QRect boundingRect, QSize imageSize)
{
    QRect frameRect;

    if(!imageSize.isNull())
    {
        double imageScale = imageSize.width() / (double)imageSize.height();
        double boudingScale = boundingRect.width() / (double)boundingRect.height();

        QRect frameRect;

        if(imageScale > boudingScale)
        {
            int top = boundingRect.top() + (boundingRect.height() - boundingRect.width() / imageScale) / 2;
            int left = boundingRect.left();

            int width = boundingRect.width();
            int height = boundingRect.width() / imageScale;

            frameRect = QRect(left, top, width, height);
        }
        else
        {
            int top = boundingRect.top();
            int left = boundingRect.left() + (boundingRect.width() - boundingRect.height() * imageScale) / 2;

            int width = boundingRect.height() * imageScale;
            int height = boundingRect.height();

            frameRect = QRect(left, top, width, height);
        }

        return frameRect;
    }

    return boundingRect;

    return frameRect;
}

QImage SImg2QImage(SImg* img)
{
    if(img == NULL || img->nW == 0)
        return QImage();

    int nByteWidth = (img->nW * 24 + 31) / 8;
    unsigned char* dst = (unsigned char*)malloc(nByteWidth * img->nH);

    for(int y = 0; y < img->nH; y ++)
    {
        for(int x = 0; x < img->nW; x ++)
        {
            dst[y * nByteWidth + x * 3] = img->pbImage[y * img->nW * 3 + x * 3 + 0];
            dst[y * nByteWidth + x * 3 + 1] = img->pbImage[y * img->nW * 3 + x * 3 + 1];
            dst[y * nByteWidth + x * 3 + 2] = img->pbImage[y * img->nW * 3 + x * 3 + 2];
        }
    }

    QImage tmpImage = QImage(dst, img->nW, img->nH, nByteWidth, QImage::Format_RGB888);
    QImage retImage = tmpImage.copy();

    free(dst);

    return retImage;
}

SImg qImage2SImg(QImage colorImage)
{
    SImg sImage = { 0 };
    if(colorImage.isNull())
        return sImage;

    if(colorImage.format() != QImage::Format_RGB888)
        colorImage = colorImage.convertToFormat(QImage::Format_RGB888);

    sImage.nW = colorImage.width();
    sImage.nH = colorImage.height();
    sImage.pbImage = (unsigned char*)malloc(sImage.nW * sImage.nH * 3);
    unsigned char* imageBuf = colorImage.bits();
    int byteWidth = colorImage.bytesPerLine();

    for(int y = 0; y < sImage.nH; y ++)
    {
        for(int x = 0; x < sImage.nW; x ++)
        {
            sImage.pbImage[(y * sImage.nW + x) * 3] = imageBuf[y * byteWidth + x * 3];
            sImage.pbImage[(y * sImage.nW + x) * 3 + 1] = imageBuf[y * byteWidth + x * 3 + 1];
            sImage.pbImage[(y * sImage.nW + x) * 3 + 2] = imageBuf[y * byteWidth + x * 3 + 2];
        }
    }

    return sImage;
}

void freeSImg(SImg* img)
{
    if(img)
    {
        if(img->pbImage)
            free(img->pbImage);
        memset(img, 0, sizeof(SImg));
    }
}

SImg createSImg(int width, int height)
{
    SImg sImage;
    sImage.nW = width;
    sImage.nH = height;
    sImage.pbImage = (unsigned char*)malloc(width * height * 3);
    return sImage;
}

void copySImg(SImg* dstImage, SImg* srcImage)
{
    if(dstImage == NULL || srcImage == NULL)
        return;

    memcpy(dstImage->pbImage, srcImage->pbImage, srcImage->nW * srcImage->nH * 3);
}

void getScaledWorkImage(QImage srcFrame, QImage& dstImage, double& scale)
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);

    if(setting.value("engine_image_size_checked1").toInt() == 0)
    {
        //dstImage = srcFrame.convertToFormat(QImage::Format_RGB888);        //image scale
        //dstImage = srcFrame.rgbSwapped();
        dstImage = srcFrame;
        scale = 1;
    }
    else
    {
        //int dst_min = setting.value("engine_image_size_value1").toInt();
        int dst_min = 100;

        scale = dst_min / (double)100;

        QMatrix matrix;
        matrix.scale(scale, scale);

        dstImage = srcFrame.transformed(matrix, Qt::FastTransformation/*Qt::SmoothTransformation*/).convertToFormat(QImage::Format_RGB888);
        dstImage = dstImage.rgbSwapped();
    }
}


//void getLogItemAndSave(QVector<LOG_ITEM>& logItems, TrackSectionResult* trackSectionResult, int sectionCount, QDateTime curDateTime, QString cameraUIDStr)
//{
//    for(int i = 0; i < sectionCount; i ++)
//    {
//        LOG_ITEM item;
//        item.dateTime = curDateTime;
//        item.name = trackSectionResult[i].szName;
//        item.gender = 1 - trackSectionResult[i].nSex;
//        item.age = trackSectionResult[i].nAge;

//        item.capturedFace = SImg2QImage(trackSectionResult[i].pmCaptured);
//        QImage sceneImage = SImg2QImage(trackSectionResult[i].pxFullImage);
//        item.logUID = QString::number(item.dateTime.toMSecsSinceEpoch()) + "-" + QString::number(i) + "_" + cameraUIDStr;
//        item.sceneImage = sceneImage;

//        logItems.append(item);
//    }
//}


QString getUserName()
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString userName = setting.value("user name").toString();
    if(userName.isEmpty())
        return "admin";
    return userName;
}

QString getPassword()
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString password = setting.value("password").toString();
    if(password.isEmpty())
        return "123456";
    return password;
}

QString getServerInfoStr()
{
    return QString::fromUtf8("Face Detection Server");
}

int getChanelCount()
{
    return 1;
}

QVector<IpCameraInfo> getIpCameraInfos()
{
    QVector<IpCameraInfo> ipCameras;
    for(int i = 0; i < getChanelCount(); i ++)
        ipCameras.append(getIpCameraInfo(i));

    return ipCameras;
}

IpCameraInfo getIpCameraInfo(int index)
{
#define DEFAULT_PORT_NUM 37777
    QString ipKeyStr, portStr, userKeyStr, passwordKeyStr;
    ipKeyStr.sprintf("ip camera chanel %d ip", index);
    portStr.sprintf("ip camera chanel %d port", index);
    userKeyStr.sprintf("ip camera chanel %d user", index);
    passwordKeyStr.sprintf("ip camera chanel %d password", index);

    QString chkFaceSizeKeyStr, detectionFaceMinSizeKeyStr, chkThresholdKeyStr, identifyThresholdKeyStr, chkBlackCandCountStr, blackCandCountStr;
    chkFaceSizeKeyStr.sprintf("ip camera channel %d chk face size", index);
    detectionFaceMinSizeKeyStr.sprintf("ip camera channel %d min face size", index);
    chkThresholdKeyStr.sprintf("ip camera channel %d chk threshold", index);
    identifyThresholdKeyStr.sprintf("ip camera channel %d identify threshold", index);
    chkBlackCandCountStr.sprintf("ip camera channel %d chk black cand count", index);
    blackCandCountStr.sprintf("ip camera channel %d black cand count", index);

    QString blackInfoSizeStr;
    blackInfoSizeStr.sprintf("ip camera channel %d black size", index);

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString ipAddress = setting.value(ipKeyStr).toString();
    int portNum = setting.value(portStr).toInt();
    if(portNum == 0)
        portNum = DEFAULT_PORT_NUM;
    QString userName = setting.value(userKeyStr).toString();
    QString password = setting.value(passwordKeyStr).toString();

    IpCameraInfo ipCameraInfo;
    ipCameraInfo.ipAddress = ipAddress;
    ipCameraInfo.portNum = portNum;
    ipCameraInfo.videoSource = userName;
    ipCameraInfo.streamUri = password;
    ipCameraInfo.streamUri = "D:/1.avi";

    ipCameraInfo.chkFaceSize = setting.value(chkFaceSizeKeyStr).toInt();
    ipCameraInfo.detectionFaceMinSize = setting.value(detectionFaceMinSizeKeyStr).toInt();
    ipCameraInfo.chkThreshold = setting.value(chkThresholdKeyStr).toInt();
    ipCameraInfo.identifyThreshold = setting.value(identifyThresholdKeyStr).toInt();
    ipCameraInfo.chkBlackCanidiateCount = setting.value(chkBlackCandCountStr).toInt();
    ipCameraInfo.blackCandidateCount = setting.value(blackCandCountStr).toInt();
    ipCameraInfo.blackInfoSize = setting.value(blackInfoSizeStr).toInt();

    return ipCameraInfo;
}

void setIpCameraInfo(int index, IpCameraInfo ipCameraInfo)
{
    QString ipKeyStr, portStr, userKeyStr, passwordKeyStr;
    ipKeyStr.sprintf("ip camera chanel %d ip", index);
    portStr.sprintf("ip camera chanel %d port", index);
    userKeyStr.sprintf("ip camera chanel %d user", index);
    passwordKeyStr.sprintf("ip camera chanel %d password", index);

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    setting.setValue(ipKeyStr, ipCameraInfo.ipAddress);
    setting.setValue(portStr, ipCameraInfo.portNum);
    setting.setValue(userKeyStr, ipCameraInfo.videoSource);
    setting.setValue(passwordKeyStr, ipCameraInfo.streamUri);
}

void setSurveillanceSetting(int index, IpCameraInfo ipCameraInfo)
{
    QString chkFaceSizeKeyStr, detectionFaceMinSizeKeyStr, chkThresholdKeyStr, identifyThresholdKeyStr, chkBlackCandCountStr, blackCandCountStr;
    chkFaceSizeKeyStr.sprintf("ip camera channel %d chk face size", index);
    detectionFaceMinSizeKeyStr.sprintf("ip camera channel %d min face size", index);
    chkThresholdKeyStr.sprintf("ip camera channel %d chk threshold", index);
    identifyThresholdKeyStr.sprintf("ip camera channel %d identify threshold", index);
    chkBlackCandCountStr.sprintf("ip camera channel %d chk black cand count", index);
    blackCandCountStr.sprintf("ip camera channel %d black cand count", index);

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    setting.setValue(chkFaceSizeKeyStr, ipCameraInfo.chkFaceSize);
    setting.setValue(detectionFaceMinSizeKeyStr, ipCameraInfo.detectionFaceMinSize);
    setting.setValue(chkThresholdKeyStr, ipCameraInfo.chkThreshold);
    setting.setValue(identifyThresholdKeyStr, ipCameraInfo.identifyThreshold);
    setting.setValue(chkBlackCandCountStr, ipCameraInfo.chkBlackCanidiateCount);
    setting.setValue(blackCandCountStr, ipCameraInfo.blackCandidateCount);
}

void setBlackInfoSize(int index, int blackInfoSize)
{
    QString blackInfoSizeStr;
    blackInfoSizeStr.sprintf("ip camera channel %d black size", index);

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    setting.setValue(blackInfoSizeStr, blackInfoSize);
}

int getHDSerial()
{
    DWORD dwVolSerial = 0;
    BOOL bIsRetrieved;
    bIsRetrieved = GetVolumeInformation(TEXT("C:\\"), NULL, NULL, &dwVolSerial, NULL, NULL, NULL, NULL);

    return dwVolSerial;
}

void freePerson1(Person1 person1)
{
    if(person1.pxFeatures)
        free(person1.pxFeatures);
}

void freePersonDatabase1(PersonDatabase1 personDatabase)
{
    if(personDatabase.pxPersons)
        free(personDatabase.pxPersons);
}

void freeAllPersonDatabase1(PersonDatabase1* personDatabase)
{
    if(personDatabase == NULL)
        return;
    for(int i = 0; i < personDatabase->nPersonNum; i ++)
    {
        if(personDatabase->pxPersons[i].pxFeatures)
            free(personDatabase->pxPersons[i].pxFeatures);

    }
    free(personDatabase->pxPersons);
    personDatabase->pxPersons = NULL;
    personDatabase->nPersonNum = 0;
}

QByteArray qImage2ByteArray(QImage image)
{
    QByteArray imageData;
    QBuffer imageBuffer(&imageData);
    imageBuffer.open(QIODevice::WriteOnly);
    image.save(&imageBuffer, "JPG");
    imageBuffer.close();

    return imageData;
}

QImage qByteArray2Image(QByteArray imageData)
{
    return QImage::fromData(imageData, "JPG");
}

QImage cropFaceImage(QImage frameImage, QRect faceRect)
{
    QRect extendFaceRect = QRect(faceRect.left() - faceRect.width() * 0.3, faceRect.top() - faceRect.height() * 0.5, faceRect.width() * 1.5, faceRect.height() * 2);
    if(extendFaceRect.left() < 0)
        extendFaceRect.setLeft(0);

    if(extendFaceRect.top() < 0)
        extendFaceRect.setTop(0);

    if(extendFaceRect.right() >= frameImage.width())
        extendFaceRect.setRight(frameImage.width() - 1);

    if(extendFaceRect.bottom() >= frameImage.height())
        extendFaceRect.setBottom(frameImage.height() - 1);
//    if((extendFaceRect.width()) % 4 != 0)
//        extendFaceRect.setWidth((extendFaceRect.width() + 3) / 4 * 4);

    QImage retImage = frameImage.copy(extendFaceRect);
    if(retImage.width() > 160 || retImage.height() > 160)
        retImage = retImage.scaled(QSize(160, 160), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return retImage;
}

QImage cropSceneImage(QImage frameImage, QRect faceRect, QRect& sceneFaceRect)
{
    QRect extendFaceRect = QRect(faceRect.left() - faceRect.width(), faceRect.top() - faceRect.height() * 0.7, faceRect.width() * 3, faceRect.height() * 4);
    if(extendFaceRect.left() < 0)
        extendFaceRect.setLeft(0);

    if(extendFaceRect.top() < 0)
        extendFaceRect.setTop(0);

    if(extendFaceRect.right() >= frameImage.width())
        extendFaceRect.setRight(frameImage.width() - 1);

    if(extendFaceRect.bottom() >= frameImage.height())
        extendFaceRect.setBottom(frameImage.height() - 1);

    QImage retImage = frameImage.copy(extendFaceRect);

    sceneFaceRect = faceRect;
    sceneFaceRect.moveTo(QPoint(faceRect.left() - extendFaceRect.left(), faceRect.top() - extendFaceRect.top()));

    return retImage;
}


QByteArray getSceneData(QString logUID)
{
    QByteArray sceneData;
    QByteArray sceneImageData;
    QByteArray sceneFeatureData;
    QRect      sceneFaceRect;

    QStringList splitList = logUID.split("_");

    if(splitList.size() != 3)
        return sceneData;

    QDateTime curDateTime = QDateTime::fromMSecsSinceEpoch(splitList[0].toLongLong());
    int chanelIndex = splitList[1].toInt();
    QString dateStr = curDateTime.date().toString("yyyy-MM-dd");
    QString chanelStr = "chanel " + QString::number(chanelIndex);

    QString sceneLogPath = getDBPath() + dateStr + "/" + chanelStr + "/_sceneLog/" + logUID + ".jpg";
    QFile dataFile(sceneLogPath);

    bool opened = dataFile.open(QIODevice::ReadOnly);
    if(opened == false)
        return sceneData;

    sceneImageData = dataFile.readAll();
    dataFile.close();

    QString featurePath = getDBPath() + dateStr + "/" + chanelStr + "/_feature/" + logUID + ".bin";

    dataFile.setFileName(featurePath);
    opened = dataFile.open(QIODevice::ReadOnly);
    if(opened == false)
        return sceneData;

    QDataStream featureStream(&dataFile);
    featureStream >> sceneFeatureData;
    featureStream >> sceneFaceRect;
    dataFile.close();

    QBuffer sendingBuffer(&sceneData);
    sendingBuffer.open(QIODevice::WriteOnly);

    QDataStream sendingDataStream(&sendingBuffer);
    sendingDataStream << sceneImageData;
    sendingDataStream << sceneFeatureData;
    sendingDataStream << sceneFaceRect;
    sendingBuffer.close();

    return sceneData;
}

QString getSceneUID(QString logUID)
{
    QStringList splitStr = logUID.split("_");
    if(splitStr.size() == 2)
        return splitStr[0];

    return logUID;
}

QString getLogUID(QString sceneUID, int index)
{
    return sceneUID + "_" + QString::number(index);
}

QString getDBPath()
{
    return LOG_DIR;
}

void setDBPath(QString dbPath)
{
    if(dbPath.isEmpty())
        return;

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    setting.setValue("db path", dbPath + "/");
}

QString getBinPath()
{
    WCHAR binPath[MAX_PATH];
    GetModuleFileName(NULL, binPath, MAX_PATH);

    QString binPathStr = QString::fromUtf16((ushort*)binPath);
    QFileInfo fileInfo(binPathStr);

    return fileInfo.absoluteDir().absolutePath();
}

QDateTime getDateTimeFromUID(QString uidStr)
{
    QDateTime logDateTime = QDateTime::currentDateTime();
    QStringList splitList = uidStr.split("_");
    if(splitList.size() == 3)
        logDateTime = QDateTime::fromMSecsSinceEpoch(splitList[0].toLongLong());

    return logDateTime;
}
