#include "logselectdlg.h"
#include "ui_logselectdlg.h"
#include "sceneimagereceivesocket.h"
#include "clientbase.h"
#include "stringtable.h"
#include "serverinfosocket.h"
#include "addblacklistfromcaptureddlg.h"

#include <QtWidgets>

LogSelectDlg::LogSelectDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogSelectDlg)
{
    ui->setupUi(this);
    retranslateUI();

    m_sceneImageReceiveSocket = NULL;
    m_selectedServerIndex = -1;
    m_selectedAreaIndex = -1;

    connect(ui->btnExportImage, SIGNAL(clicked()), this, SLOT(slotExportImage()));
    connect(ui->btnAddToBlackList, SIGNAL(clicked()), this, SLOT(slotAddToBlackList()));
}

LogSelectDlg::~LogSelectDlg()
{
    destroySceneSocket();

    delete ui;
}

void LogSelectDlg::setInfo(QVector<MonitoringAreaInfo> monitoringAreaInfos, QVector<ServerInfoSocket*> serverInfoSockets, LOG_RESULT logResult)
{
    m_monitoringAreaInfos = monitoringAreaInfos;
    m_serverInfoSockets = serverInfoSockets;
    m_faceImageInfo = logResult;

    ui->editDateTime->setText(logResult.dateTime.toString("yyyy-MM-dd hh:mm:ss"));

    int serverIndex = getServerIndexFromUID(serverInfoSockets, logResult.serverUID);
    int areaIndex = getAreaIndexFromServerIndex(monitoringAreaInfos, serverIndex, logResult.chanelIndex);
    if(areaIndex < 0 || serverIndex < 0)
        return;

//    QString serverInfoStr;
//    serverInfoStr.sprintf("%s - %s (%s %d)", logResult.areaName.toUtf8().data(),
//                          getServerNameFromUID(serverInfoSockets, logResult.serverUID).toUtf8().data(),
//                          StringTable::Str_Chanel.toUtf8().data(), logResult.chanelIndex + 1);

    ui->editServerInfo->setText(logResult.areaName);

    ui->lblScore->hide();
    ui->groupBlackList->hide();
    destroySceneSocket();
    m_sceneImageReceiveSocket = new SceneImageReceiveSocket;
    connect(m_sceneImageReceiveSocket, SIGNAL(receivedSceneData(FRAME_RESULT, QImage)), this, SLOT(slotReceiveSceneImage(FRAME_RESULT, QImage)));

    m_sceneImageReceiveSocket->startSocket(m_serverInfoSockets[serverIndex]->serverInfo(), m_faceImageInfo.logUID, 0);

    m_selectedServerIndex = serverIndex;
    m_selectedAreaIndex = areaIndex;
}

void LogSelectDlg::setInfo(QVector<MonitoringAreaInfo> monitoringAreaInfos, QVector<ServerInfoSocket*> serverInfoSockets, QVector<BLACK_RECOG_RESULT> blackRecogResult)
{
    if(blackRecogResult.size() == 0)
        return;

    m_monitoringAreaInfos = monitoringAreaInfos;
    m_serverInfoSockets = serverInfoSockets;
    m_blackRecogResult = blackRecogResult;

    ui->editDateTime->setText(m_blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));

    int serverIndex = getServerIndexFromUID(serverInfoSockets, m_blackRecogResult[0].serverUID);
    int areaIndex = getAreaIndexFromServerIndex(monitoringAreaInfos, serverIndex, m_blackRecogResult[0].chanelIndex);
    if(areaIndex < 0 || serverIndex < 0)
        return;

//    QString serverInfoStr;
//    serverInfoStr.sprintf("%s - %s (%s %d)", blackRecogResult[0].areaName.toUtf8().data(), m_serverInfoSockets[serverIndex]->serverInfo().serverName.toUtf8().data(),
//            StringTable::Str_Chanel.toUtf8().data(), m_blackRecogResult[0].chanelIndex + 1);
    ui->editServerInfo->setText(blackRecogResult[0].areaName);

    QString scoreStr;
    scoreStr.sprintf("%s : %f", StringTable::Str_Similiarity.toUtf8().data(), blackRecogResult[0].similiarity);
    ui->lblScore->setText("<font color='#FF0000'>" + scoreStr + "</font>");

    destroySceneSocket();
    m_sceneImageReceiveSocket = new SceneImageReceiveSocket;
    connect(m_sceneImageReceiveSocket, SIGNAL(receivedSceneData(FRAME_RESULT, QImage)), this, SLOT(slotReceiveSceneImage(FRAME_RESULT, QImage)));

    m_sceneImageReceiveSocket->startSocket(m_serverInfoSockets[serverIndex]->serverInfo(), m_blackRecogResult[0].logUID, 1);

    m_selectedServerIndex = serverIndex;
    m_selectedAreaIndex = areaIndex;
}

void LogSelectDlg::destroySceneSocket()
{
    if(m_sceneImageReceiveSocket)
    {
        m_sceneImageReceiveSocket->stopSocket();
        delete m_sceneImageReceiveSocket;
        m_sceneImageReceiveSocket = NULL;
    }
}

void LogSelectDlg::updateFaceImages()
{
    if(m_blackRecogResult.size())
    {
        QImage probeImage = qByteArray2Image(m_blackRecogResult[0].probeFaceData);
        QImage galleryImage = qByteArray2Image(m_blackRecogResult[0].galleryFaceData);

        QRect probeImageRect = getFrameRect(ui->lblCaptured->rect(), probeImage.size());
        QImage probeScaledImage = probeImage.scaled(probeImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QRect galleryImageRect = getFrameRect(ui->lblBlackList->rect(), galleryImage.size());
        QImage galleryScaledImage = galleryImage.scaled(galleryImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QPainter painter;
        painter.begin(&galleryScaledImage);
        if(m_blackRecogResult[0].personType == 0)
            painter.setPen(QPen(QColor(0, 255, 0), 3));
        else
            painter.setPen(QPen(QColor(255, 0, 0), 3));
        painter.drawRect(QRect(1, 1, galleryScaledImage.size().width() - 3, galleryScaledImage.size().height() - 3));
        painter.end();


        ui->lblCaptured->setPixmap(QPixmap::fromImage(probeScaledImage));
        ui->lblBlackList->setPixmap(QPixmap::fromImage(galleryScaledImage));
    }
    else
    {
        QImage faceImage = QImage::fromData(m_faceImageInfo.faceImage);
        QRect probeImageRect = getFrameRect(ui->lblCaptured->rect(), faceImage.size());
        QImage probeScaledImage = faceImage.scaled(probeImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        ui->lblCaptured->setPixmap(QPixmap::fromImage(probeScaledImage));
    }

}

void LogSelectDlg::slotReceiveSceneImage(FRAME_RESULT frameResult, QImage sceneImage)
{
    QRect sceneImageRect = getFrameRect(ui->lblScene->rect(), sceneImage.size());
    QImage scaledImage = sceneImage.scaled(sceneImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPainter painter;
    painter.begin(&scaledImage);

    QRect faceRect = QRect(frameResult.faceRect.left() * scaledImage.width() / (float)sceneImage.width(),
                     frameResult.faceRect.top() * scaledImage.height() / (float)sceneImage.height(),
                     frameResult.faceRect.width() * scaledImage.width() / (float)sceneImage.width(),
                     frameResult.faceRect.height() * scaledImage.height() / (float)sceneImage.height());

    painter.setPen(Qt::green);
    painter.drawRect(faceRect);

    painter.end();

    ui->lblScene->setPixmap(QPixmap::fromImage(scaledImage));

    m_sceneImage = sceneImage;
    m_frameResult = frameResult;
}

void LogSelectDlg::slotAddToBlackList()
{
    if(m_blackRecogResult.size())
    {
        AddBlackListFromCapturedDlg dlg;
        dlg.setInfo(m_monitoringAreaInfos, m_serverInfoSockets, m_selectedAreaIndex, m_blackRecogResult[0].chanelIndex, m_sceneImage, 0);
        dlg.addFrameResult(m_frameResult);
        dlg.exec();
    }
    else
    {
        AddBlackListFromCapturedDlg dlg;
        dlg.setInfo(m_monitoringAreaInfos, m_serverInfoSockets, m_selectedAreaIndex, m_faceImageInfo.chanelIndex, m_sceneImage, 0);
        dlg.addFrameResult(m_frameResult);
        dlg.exec();
    }
}

void LogSelectDlg::slotExportImage()
{
    if(m_selectedAreaIndex < 0 || m_selectedServerIndex < 0)
        return;

    if(m_blackRecogResult.size())
    {
        QString saveFileName;
        saveFileName.sprintf("%s(%s%d) %s", m_serverInfoSockets[m_selectedServerIndex]->serverInfo().serverName.toUtf8().data(),
                             StringTable::Str_Chanel.toUtf8().data(),
                             m_blackRecogResult[0].chanelIndex + 1,
                m_blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh-mm-ss").toUtf8().data());

        QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
        QString selectedFileName = QFileDialog::getSaveFileName(this, StringTable::Str_Save_File, setting.value("export image dir").toString() + saveFileName, "Image (*.jpg)");
        if(selectedFileName.isEmpty())
            return;

        QFileInfo info(selectedFileName);
        setting.setValue("export image dir", info.absoluteDir().absolutePath());
        m_sceneImage.save(selectedFileName, "JPG");
    }
    else
    {
        QString saveFileName;
        saveFileName.sprintf("%s(%s%d) %s", m_serverInfoSockets[m_selectedServerIndex]->serverInfo().serverName.toUtf8().data(),
                             StringTable::Str_Chanel.toUtf8().data(),
                             m_faceImageInfo.chanelIndex + 1,
                             m_faceImageInfo.dateTime.toString("yyyy-MM-dd hh-mm-ss").toUtf8().data());

        QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
        QString selectedFileName = QFileDialog::getSaveFileName(this, StringTable::Str_Save_File, setting.value("export image dir").toString() + saveFileName, "Image (*.jpg)");
        if(selectedFileName.isEmpty())
            return;

        QFileInfo info(selectedFileName);
        setting.setValue("export image dir", info.absoluteDir().absolutePath());
        m_sceneImage.save(selectedFileName, "JPG");
    }
}


void LogSelectDlg::resizeEvent(QResizeEvent* e)
{
    QDialog::resizeEvent(e);
    updateFaceImages();
}

void LogSelectDlg::paintEvent(QPaintEvent *)
{
    updateFaceImages();
}

void LogSelectDlg::retranslateUI()
{
    ui->groupCaptured->setTitle(StringTable::Str_Captured);
    ui->groupBlackList->setTitle(StringTable::Str_Black_DB);
    ui->groupInfo->setTitle(StringTable::Str_Info);
    ui->groupManage->setTitle(StringTable::Str_Manage);
    ui->groupFullImage->setTitle(StringTable::Str_Full_Image);
    ui->btnAddToBlackList->setText(StringTable::Str_Add_to_black_list);
    ui->btnExportImage->setText(StringTable::Str_Export_image);

    setWindowTitle(StringTable::Str_Log_Detail);

}
