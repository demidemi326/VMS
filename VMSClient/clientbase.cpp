#include "clientbase.h"
#include "serverinfosocket.h"
#include "socketbase.h"

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

}

SImg qImage2SImg(QImage colorImage)
{
    SImg sImage = { 0 };
    if(colorImage.isNull())
        return sImage;

    sImage.nW = colorImage.width();
    sImage.nH = colorImage.height();
    sImage.pbImage = (unsigned char*)malloc(sImage.nW * sImage.nH * 3);
    unsigned char* imageBuf = colorImage.bits();
    int byteWidth = colorImage.bytesPerLine();
    int nStep = colorImage.format() == QImage::Format_RGB888 ? 3 : 4;

    if(colorImage.format() == QImage::Format_Indexed8)
    {
        for(int y = 0; y < sImage.nH; y ++)
        {
            for(int x = 0; x < sImage.nW; x ++)
            {
                sImage.pbImage[(y * sImage.nW + x) * 3] = imageBuf[y * byteWidth + x];
                sImage.pbImage[(y * sImage.nW + x) * 3 + 1] = imageBuf[y * byteWidth + x];
                sImage.pbImage[(y * sImage.nW + x) * 3 + 2] = imageBuf[y * byteWidth + x];
            }
        }
    }
    else
    {
        for(int y = 0; y < sImage.nH; y ++)
        {
            for(int x = 0; x < sImage.nW; x ++)
            {
                sImage.pbImage[(y * sImage.nW + x) * 3] = imageBuf[y * byteWidth + x * nStep];
                sImage.pbImage[(y * sImage.nW + x) * 3 + 1] = imageBuf[y * byteWidth + x * nStep + 1];
                sImage.pbImage[(y * sImage.nW + x) * 3 + 2] = imageBuf[y * byteWidth + x * nStep + 2];
            }
        }
    }

    return sImage;
}

void freeSImg(SImg sImage)
{
    if(sImage.pbImage)
        free(sImage.pbImage);
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

    QImage retImage = frameImage.copy(extendFaceRect);
    if(retImage.width() > 160 || retImage.height() > 160)
        retImage = retImage.scaled(QSize(160, 160), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return retImage;
}


int serverType()
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    int serverType = setting.value("server type").toInt();

    return serverType;
}


int getServerIndexFromUID(QVector<ServerInfoSocket*> serverInfoSockets, int serverUID)
{
    int existServer = -1;
    for(int i = 0; i < serverInfoSockets.size(); i ++)
    {
        if(serverInfoSockets[i]->serverInfo().serverUID == serverUID)
        {
            existServer = i;
            break;
        }
    }

    return existServer;
}

int getAreaIndexFromServerIndex(QVector<MonitoringAreaInfo> areaInfo, int serverIndex, int chanelIndex)
{
    int existArea = -1;
    for(int i = 0; i < areaInfo.size(); i ++)
    {
        for(int j = 0; j < areaInfo[i].serverIndexs.size(); j ++)
        {
            if(areaInfo[i].serverIndexs[j] == serverIndex &&
                    areaInfo[i].chanelIndexs[j] == chanelIndex)
            {
                existArea = i;
                break;
            }
        }
    }

    return existArea;
}

QString getServerNameFromUID(QVector<ServerInfoSocket*> serverInfoSockets, int serverUID)
{
    QString serverName;
    for(int i = 0; i < serverInfoSockets.size(); i ++)
    {
        if(serverInfoSockets[i]->serverInfo().serverUID == serverUID)
        {
            serverName = serverInfoSockets[i]->serverInfo().serverName;
            break;
        }
    }
    return serverName;
}

QString getAreaNameFromUID(QVector<MonitoringAreaInfo> areaInfos, QVector<ServerInfoSocket*> serverInfoSockets, int serverUID, int chanelIndex)
{
    QString serverName = "-";

    int serverIndex = -1;
    for(int i = 0; i < serverInfoSockets.size(); i ++)
    {
        if(serverInfoSockets[i]->serverInfo().serverUID == serverUID)
        {
            serverIndex = i;
            break;
        }
    }

    for(int i = 0; i < areaInfos.size(); i ++)
    {
        for(int j = 0; j < areaInfos[i].serverIndexs.size(); j ++)
        {
            if(serverIndex == areaInfos[i].serverIndexs[j] && chanelIndex == areaInfos[i].chanelIndexs[j])
            {
                return areaInfos[i].areaName;
            }
        }
    }

    return serverName;
}

QString readPass()
{
    QString pass;
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    pass = setting.value("password").toString();

    return pass;
}

void writePass(QString pass)
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    setting.setValue("password", pass);
}
