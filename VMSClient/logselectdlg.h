#ifndef LOGSELECTDLG_H
#define LOGSELECTDLG_H

#include "clientbase.h"
#include <QDialog>

namespace Ui {
class LogSelectDlg;
}

class ServerInfoSocket;
class SceneImageReceiveSocket;
class LogSelectDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LogSelectDlg(QWidget *parent = 0);
    ~LogSelectDlg();

    void    setInfo(QVector<MonitoringAreaInfo> monitoringAreaInfos, QVector<ServerInfoSocket*> serverInfoSockets, LOG_RESULT faceImageInfo);
    void    setInfo(QVector<MonitoringAreaInfo> monitoringAreaInfos, QVector<ServerInfoSocket*> serverInfoSockets, QVector<BLACK_RECOG_RESULT> blackRecogResult);

public slots:
    void    slotReceiveSceneImage(FRAME_RESULT frameResult, QImage sceneImage);

    void    slotAddToBlackList();
    void    slotExportImage();

private:
    void    destroySceneSocket();
    void    updateFaceImages();
    void    retranslateUI();

protected:
    void    resizeEvent(QResizeEvent* e);
    void    paintEvent(QPaintEvent *);

private:
    Ui::LogSelectDlg *ui;

    QVector<MonitoringAreaInfo> m_monitoringAreaInfos;
    QVector<ServerInfoSocket*>  m_serverInfoSockets;
    int                         m_selectedServerIndex;
    int                         m_selectedAreaIndex;
    LOG_RESULT m_faceImageInfo;
    QVector<BLACK_RECOG_RESULT> m_blackRecogResult;

    SceneImageReceiveSocket*    m_sceneImageReceiveSocket;
    QImage      m_sceneImage;
    FRAME_RESULT    m_frameResult;
};

#endif // LOGSELECTDLG_H
