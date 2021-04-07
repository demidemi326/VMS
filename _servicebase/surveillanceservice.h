#ifndef SURVEILLANCESERVICE_H
#define SURVEILLANCESERVICE_H

#include <QObject>
#include <QVector>

#include "servicebase.h"
#include "socketbase.h"

class DongleFinder;
class CameraProcessEngine;
class ClientListeningSocket;
class ServerInfoSocket;
class FaceSendingSocket;
class FaceImageSendingSocket;
class AuthenticatingSocket;
class SceneImageSendSocket;
class LogSaveThread;
class ImageProcessingSocket;
class AddBlackInfoReceiveSocket;
class BlackInfoSendingSocket;
class OneBlackPersonEditSocket;
class BlackPersonDeleteSocket;
class BlackRecogResultSendingSocket;
class SearchLogSendingSocket;
class SearchBlackSendingSocket;
class LogSaveToSqlThread;
class FaceDetectionEngine;
class ModelDetectionEngine;
class LogCheckThread;
class SearchLastBlackSendingSocket;
class SurveillanceService : public QObject
{
    Q_OBJECT
public:
    explicit SurveillanceService(QObject *parent = 0);

    void    create();
    void    release();

    void    start();
    void    stop();

signals:
    void    logOut(QString);

public slots:
    void    slotNewClientConnection(SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void    slotAuthenticatedSocket(int socketType, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void    slotReceiveIpCameraInfos(QVector<IpCameraInfo> cameraInfos);
    void    slotReceiveSurveillanceSetting(IpCameraInfo surveillanceSetting, int chanelIndex);
    void    slotReceiveIpCameraInfo(IpCameraInfo ipCameraInfo, int chanelIndex);

    void    slotAuthenticateingFinished(QObject*);
    void    slotFaceSendingSocketError(QObject* obj);
    void    slotFaceImageSendingSocketError(QObject* obj);
    void    slotSceneImageSendFinished(QObject*);
    void    slotImageProcessingFinished(QObject*);
    void    slotLogChanged(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults, QVector<LOG_ITEM> logItem, int chanelIndex);
    void    slotLogSaveFinished(QObject* obj);
    void    slotReceiveBlackPersonFinished(QObject* obj);
    void    slotBlackInfoSendingFinished(QObject* obj);
    void    slotReceiveEditBlackInfoFinished(QObject*);
    void    slotReceiveDeleteBlackInfoFinished(QObject*);
    void    slotBlackRecogResultSendSocketError(QObject* obj);
    void    slotSearchLogSendingSocketFinished(QObject* obj);
    void    slotSearchBlackSendingSocketFinished(QObject* obj);
    void    slotSearchLastBlackSendingSocketFinished(QObject* obj);

    void    slotLogOut(QString logOutStr);

private:
    void    registerMetaType();
    void    setupTVideograbber();
    void    setupEngines();

    void    setupCameras();
    void    releaseCameras();

    void    setupSockets();
    void    releaseSockets();

private:
    int     m_running;

    QVector<CameraProcessEngine*>   m_cameraProcessEngines;
    DongleFinder*                   m_dongleFindger;
//    QVector<FaceDetectionEngine*>   m_faceDetectionEngines;
//    QVector<ModelDetectionEngine*>  m_modelDetectionEngines;

    LogCheckThread*                 m_logCheckThread;

    ClientListeningSocket*          m_clientListeningSocket;
    QVector<AuthenticatingSocket*>     m_authenticatingSockets;
    QVector<ServerInfoSocket*>      m_serverInfoSockets;
    QVector<FaceSendingSocket*>     m_faceSendingSockets;
    QVector<FaceImageSendingSocket*>     m_faceImageSendingSockets;
    QVector<SceneImageSendSocket*>     m_sceneImageSendSockets;
    QVector<LogSaveThread*>         m_logSaveThread;
    QVector<ImageProcessingSocket*> m_imageProcessingSockets;
    QVector<AddBlackInfoReceiveSocket*> m_oneBlackPersonReceiveSockets;
    QVector<BlackInfoSendingSocket*>      m_blackInfoSendingSockets;
    QVector<OneBlackPersonEditSocket*>    m_oneBlackPersonEditSockets;
    QVector<BlackPersonDeleteSocket*>     m_blackPersonDeleteSockets;
    QVector<BlackRecogResultSendingSocket*>     m_blackRecogResultSendingSockets;
    QVector<SearchLogSendingSocket*>            m_searchLogSendingSockets;
    QVector<SearchBlackSendingSocket*>          m_searchBlckSendingSockets;
    QVector<SearchLastBlackSendingSocket*>          m_searchLastBlckSendingSockets;

    QMutex                          m_authenticatingMutex;
};

#endif // SURVEILLANCESERVICE_H
