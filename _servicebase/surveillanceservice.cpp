#include "surveillanceservice.h"
#include "servicebase.h"
#include "cameraprocessengine.h"
#include "clientlisteningsocket.h"
#include "socketbase.h"
#include "authenticatingsocket.h"
#include "serverinfosocket.h"
#include "facesendingsocket.h"
#include "faceimagesendingsocket.h"
#include "sceneimagesendsocket.h"
#include "logsavethread.h"
#include "imageprocessingsocket.h"
#include "addblackinforeceivesocket.h"
#include "blackinfosendingsocket.h"
#include "oneblackpersoneditsocket.h"
#include "blackpersondeletesocket.h"
#include "blackrecogresultsendingsocket.h"
#include "searchlogsendingsocket.h"
#include "searchblacksendingsocket.h"
#include "logcheckthread.h"
#include "searchlastblacksendingsocket.h"
#include "ipcamera.h"
#include "donglefinder.h"

#include <QtWidgets>

SurveillanceService::SurveillanceService(QObject *parent) :
    QObject(parent)
{
    m_running = 0;
    m_clientListeningSocket = NULL;
    m_logCheckThread = NULL;
    m_dongleFindger = NULL;
}

void SurveillanceService::create()
{
    registerMetaType();
    setupEngines();
    IpCamera::init();
    if(m_dongleFindger == NULL)
        m_dongleFindger = new DongleFinder;
}

void SurveillanceService::release()
{
    stop();

    if(m_dongleFindger)
    {
        m_dongleFindger->stopFind();
        delete m_dongleFindger;
        m_dongleFindger = NULL;
    }
    ReleaseEngine();
}

void SurveillanceService::start()
{
    if(m_running)
        return;

    m_running = 1;

    if(m_logCheckThread == NULL)
    {
        m_logCheckThread = new LogCheckThread;
        m_logCheckThread->startThread(getDBPath() + "/log_info.db");
    }

    setupCameras();
    setupSockets();

    m_dongleFindger->start();
}

void SurveillanceService::stop()
{
    if(m_running == 0)
        return;

    m_dongleFindger->stopFind();

    releaseSockets();

    releaseCameras();

    m_logCheckThread->stopThread();
    delete m_logCheckThread;
    m_logCheckThread = NULL;

    m_running = 0;
}

void SurveillanceService::registerMetaType()
{
    qRegisterMetaType<QVector<QRect> >("QVector<QRect>");
    qRegisterMetaType<IpCameraInfo>("IpCameraInfo");
    qRegisterMetaType<SOCKET>("SOCKET");
    qRegisterMetaType<SImg>("SImg");
    qRegisterMetaType<SOCKADDR_IN>("SOCKADDR_IN");
    qRegisterMetaType<QVector<LOG_ITEM> >("QVector<LOG_ITEM>");
    qRegisterMetaType<SCENE_LOG_ITEM>("SCENE_LOG_ITEM");
    qRegisterMetaType<QVector<IpCameraInfo> >("QVector<IpCameraInfo>");
    qRegisterMetaType<Person1>("Person1");
    qRegisterMetaType<PersonDatabase1>("PersonDatabase1");
    qRegisterMetaType<BlackPersonMetaInfo>("BlackPersonMetaInfo");
    qRegisterMetaType<QVector<QVector<BLACK_RECOG_INFO> > >("QVector<QVector<BLACK_RECOG_INFO> >");
    qRegisterMetaType<QVector<DetectionResult> >("QVector<DetectionResult>");
    qRegisterMetaType<QVector<LOG_ITEM_INFO> >("QVector<LOG_ITEM_INFO>");
}

void SurveillanceService::setupTVideograbber()
{
}

void SurveillanceService::setupEngines()
{
    qDebug() << "creat engine" << getBinPath();
    int ret = CreateEngine(getBinPath().toUtf8().data(), "./enroll_track.db");
    if(ret == 0)
        qApp->quit();
    qDebug() << "ret" << ret << getBinPath();
}

void SurveillanceService::setupCameras()
{
    QVector<IpCameraInfo> ipCameraInfo = getIpCameraInfos();
    for(int i = 0; i < ipCameraInfo.size(); i ++)
    {
        CameraProcessEngine* cameraProcessEngine = new CameraProcessEngine;
        m_cameraProcessEngines.append(cameraProcessEngine);

        connect(cameraProcessEngine, SIGNAL(logOut(QString)), this, SLOT(slotLogOut(QString)));
        connect(cameraProcessEngine, SIGNAL(usbCheckError()), qApp, SLOT(quit()));
        cameraProcessEngine->startProcess(i, ipCameraInfo[i]);
    }
}

void SurveillanceService::releaseCameras()
{
    for(int i = 0; i < m_cameraProcessEngines.size(); i ++)
    {
        m_cameraProcessEngines[i]->stopProcess();
        delete m_cameraProcessEngines[i];
    }
    m_cameraProcessEngines.clear();
}

void SurveillanceService::setupSockets()
{
    m_clientListeningSocket = new ClientListeningSocket;

    connect(m_clientListeningSocket, SIGNAL(newConnection(SOCKET,SOCKADDR_IN,int)), this, SLOT(slotNewClientConnection(SOCKET,SOCKADDR_IN,int)));

    m_clientListeningSocket->startListening(VMS_SERVER_PORT);
}

void SurveillanceService::releaseSockets()
{
    for(int i = 0; i < m_authenticatingSockets.size(); i ++)
    {
        m_authenticatingSockets[i]->stopAuthenticate();
        delete m_authenticatingSockets[i];
    }
    m_authenticatingSockets.clear();

    for(int i = 0; i < m_faceSendingSockets.size(); i ++)
    {
        m_faceSendingSockets[i]->stopSocket();
        delete m_faceSendingSockets[i];
    }
    m_faceSendingSockets.clear();

    for(int i = 0; i < m_faceImageSendingSockets.size(); i ++)
    {
        m_faceImageSendingSockets[i]->stopSocket();
        delete m_faceImageSendingSockets[i];
    }
    m_faceImageSendingSockets.clear();

    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        m_serverInfoSockets[i]->stopSocket();
        delete m_serverInfoSockets[i];
    }

    m_serverInfoSockets.clear();


    for(int i = 0; i < m_blackPersonDeleteSockets.size(); i ++)
    {
        m_blackPersonDeleteSockets[i]->stopSocket();
        delete m_blackPersonDeleteSockets[i];
    }
    m_blackPersonDeleteSockets.clear();

    for(int i = 0; i < m_searchLogSendingSockets.size(); i ++)
    {
        m_searchLogSendingSockets[i]->stopSocket();
        delete m_searchLogSendingSockets[i];
    }
    m_searchLogSendingSockets.clear();

    for(int i = 0; i < m_searchBlckSendingSockets.size(); i ++)
    {
        m_searchBlckSendingSockets[i]->stopSocket();
        delete m_searchBlckSendingSockets[i];
    }
    m_searchBlckSendingSockets.clear();

    for(int i = 0; i < m_searchLastBlckSendingSockets.size(); i ++)
    {
        m_searchLastBlckSendingSockets[i]->stopSocket();
        delete m_searchLastBlckSendingSockets[i];
    }
    m_searchLastBlckSendingSockets.clear();

    if(m_clientListeningSocket)
    {
        m_clientListeningSocket->stopListening();
        delete m_clientListeningSocket;
        m_clientListeningSocket = NULL;
    }
}

void SurveillanceService::slotNewClientConnection(SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    AuthenticatingSocket* authenticatingSocket = new AuthenticatingSocket;
    connect(authenticatingSocket, SIGNAL(authenticatedSocket(int,SOCKET,SOCKADDR_IN,int)), this, SLOT(slotAuthenticatedSocket(int,SOCKET, SOCKADDR_IN, int)));
    connect(authenticatingSocket, SIGNAL(authenticateFinished(QObject*)), this, SLOT(slotAuthenticateingFinished(QObject*)));

    m_authenticatingSockets.append(authenticatingSocket);

    authenticatingSocket->startAuthenticating(socket, socketIn, socketInLen);
}

void SurveillanceService::slotAuthenticatedSocket(int socketType, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    if(socketType == LISTEN_TYPE_SERVER_INFO)
    {
        ServerInfoSocket* serverInfoSocket = new ServerInfoSocket;
        serverInfoSocket->setCameraProcessEngines(m_cameraProcessEngines);
        m_serverInfoSockets.append(serverInfoSocket);

        connect(serverInfoSocket, SIGNAL(receiveIpCameraInfos(QVector<IpCameraInfo>)), this, SLOT(slotReceiveIpCameraInfos(QVector<IpCameraInfo>)));
        connect(serverInfoSocket, SIGNAL(receiveSurveillanceSetting(IpCameraInfo, int)), this, SLOT(slotReceiveSurveillanceSetting(IpCameraInfo, int)));
        connect(serverInfoSocket, SIGNAL(receiveIpCameraInfo(IpCameraInfo, int)), this, SLOT(slotReceiveIpCameraInfo(IpCameraInfo, int)));
        for(int i = 0; i < m_cameraProcessEngines.size(); i ++)
            connect(m_cameraProcessEngines[i], SIGNAL(blackInfoChanged()), serverInfoSocket, SLOT(slotBlackInfoChanged()));

        serverInfoSocket->startSocket(socket, socketIn, socketInLen);
    }
    else if(socketType == LISTEN_TYPE_FACE_RESULT)
    {
        int chanelIndex = socketInLen;
        if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
            m_cameraProcessEngines[chanelIndex]->addFaceRectSendingSocket(socket);
    }
    else if(socketType == LISTEN_TYPE_FACE_IMAGE)
    {
        int chanelIndex = socketInLen;
        if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
            m_cameraProcessEngines[chanelIndex]->addLogSendingSocket(socket);
    }
    else if(socketType == LISTEN_TYPE_SCENE_IMAGE)
    {
        SceneImageSendSocket* sceneImageSendSocket = new SceneImageSendSocket;
        m_sceneImageSendSockets.append(sceneImageSendSocket);

        connect(sceneImageSendSocket, SIGNAL(sendFinshed(QObject*)), this, SLOT(slotSceneImageSendFinished(QObject*)));

        sceneImageSendSocket->setSocketInfo(socket, socketIn, socketInLen);
    }
    else if(socketType == LISTEN_TYPE_IMAGE_PROCESSING)
    {
        ImageProcessingSocket* imageProcessingSocket = new ImageProcessingSocket;
        imageProcessingSocket->setSocketInfo(m_imageProcessingSockets.size() + m_cameraProcessEngines.size(),socket, socketIn, socketInLen);

        connect(imageProcessingSocket, SIGNAL(processingFinshed(QObject*)), this, SLOT(slotImageProcessingFinished(QObject*)));

        m_imageProcessingSockets.append(imageProcessingSocket);
    }
    else if(socketType == LISTEN_TYPE_ONE_BLACK)
    {
        int chanelIndex = socketInLen;
        AddBlackInfoReceiveSocket* oneBlackPersonReceiveSocket = new AddBlackInfoReceiveSocket;
        m_oneBlackPersonReceiveSockets.append(oneBlackPersonReceiveSocket);
        if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
        {
            connect(oneBlackPersonReceiveSocket, SIGNAL(receiveBlackPersoInfo(Person1,BlackPersonMetaInfo)), m_cameraProcessEngines[chanelIndex], SLOT(slotAddOneBlackPerson(Person1,BlackPersonMetaInfo)));
            connect(oneBlackPersonReceiveSocket, SIGNAL(receivedBlackPersonData(QByteArray)), m_cameraProcessEngines[chanelIndex], SLOT(slotAddBlackPersonData(QByteArray)));
        }

        connect(oneBlackPersonReceiveSocket, SIGNAL(receiveFinished(QObject*)), this, SLOT(slotReceiveBlackPersonFinished(QObject*)));

        oneBlackPersonReceiveSocket->setSocketInfo(chanelIndex, socket, socketIn, sizeof(SOCKADDR_IN));
    }
    else if(socketType ==  LISTEN_TYPE_BLACK_INFO)
    {
        int chanelIndex = socketInLen;
        BlackInfoSendingSocket* blackInfoSendingSocket = new BlackInfoSendingSocket;
        m_blackInfoSendingSockets.append(blackInfoSendingSocket);

        connect(blackInfoSendingSocket, SIGNAL(sendingFinished(QObject*)), this, SLOT(slotBlackInfoSendingFinished(QObject*)));

        blackInfoSendingSocket->setSocketInfo(chanelIndex, socket, socketIn, sizeof(SOCKADDR_IN));
        if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
            blackInfoSendingSocket->sendBlackInfo(m_cameraProcessEngines[chanelIndex]->blackPersonDatabase(), m_cameraProcessEngines[chanelIndex]->blackPersonMeataInfo());
    }
    else if(socketType == LISTEN_TYPE_EDIT_ONE_BLACK)
    {
        int chanelIndex = socketInLen;
        OneBlackPersonEditSocket* oneBlackEditSocket = new OneBlackPersonEditSocket;
        m_oneBlackPersonEditSockets.append(oneBlackEditSocket);

        connect(oneBlackEditSocket, SIGNAL(receiveEditInfoFinished(QObject*)), this, SLOT(slotReceiveEditBlackInfoFinished(QObject*)));
        if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
        {
            connect(oneBlackEditSocket, SIGNAL(receiveEditBlackPersonInfo(QString, Person1, BlackPersonMetaInfo)),
                    m_cameraProcessEngines[chanelIndex], SLOT(slotEditOneBlackPerson(QString, Person1, BlackPersonMetaInfo)));
        }

        oneBlackEditSocket->setSocketInfo(chanelIndex, socket, socketIn, socketInLen);
    }
    else if(socketType == LISTEN_TYPE_DELETE_BLACK)
    {
        int chanelIndex = socketInLen;
        BlackPersonDeleteSocket* blackDeleteSocket = new BlackPersonDeleteSocket;
        m_blackPersonDeleteSockets.append(blackDeleteSocket);

        connect(blackDeleteSocket, SIGNAL(receivedDeleteInfoFinished(QObject*)), this, SLOT(slotReceiveDeleteBlackInfoFinished(QObject*)));
        if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
        {
            connect(blackDeleteSocket, SIGNAL(receiveDeleteBlackPersonInfo(QStringList)),
                    m_cameraProcessEngines[chanelIndex], SLOT(slotDeleteBlackInfos(QStringList)));
        }

        blackDeleteSocket->setSocketInfo(chanelIndex, socket, socketIn, socketInLen);
    }
    else if(socketType == LISTEN_TYPE_BLACK_RECOG_RESULT)
    {
        int chanelIndex = socketInLen;
        if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
            m_cameraProcessEngines[chanelIndex]->addBlackSendingSocket(socket);
    }
    else if(socketType == LISTEN_TYPE_SEARCH_LOG)
    {
        int chanelIndex = socketInLen;

        SearchLogSendingSocket* searchLogSendingSocket = new SearchLogSendingSocket;
        m_searchLogSendingSockets.append(searchLogSendingSocket);

        connect(searchLogSendingSocket, SIGNAL(socketError(QObject*)), this, SLOT(slotSearchLogSendingSocketFinished(QObject*)));

        searchLogSendingSocket->startSocket(chanelIndex, socket, socketIn, sizeof(SOCKADDR_IN));
    }
    else if(socketType == LISTEN_TYPE_SEARCH_BLACK)
    {
        int chanelIndex = socketInLen;

        SearchBlackSendingSocket* searchBlackSendingSocket = new SearchBlackSendingSocket;
        m_searchBlckSendingSockets.append(searchBlackSendingSocket);

        connect(searchBlackSendingSocket, SIGNAL(socketError(QObject*)), this, SLOT(slotSearchBlackSendingSocketFinished(QObject*)));

        searchBlackSendingSocket->startSocket(chanelIndex, socket, socketIn, sizeof(SOCKADDR_IN));
    }
    else if(socketType == LISTEN_TYPE_SEARCH_LAST_BLACK)
    {
        int chanelIndex = socketInLen;

        SearchLastBlackSendingSocket* searchLastBlackSendingSocket = new SearchLastBlackSendingSocket;
        searchLastBlackSendingSocket->setSavedLogDate(m_logCheckThread->logSavedDate());
        m_searchLastBlckSendingSockets.append(searchLastBlackSendingSocket);

        connect(searchLastBlackSendingSocket, SIGNAL(socketError(QObject*)), this, SLOT(slotSearchLastBlackSendingSocketFinished(QObject*)));

        searchLastBlackSendingSocket->startSocket(chanelIndex, socket, socketIn, sizeof(SOCKADDR_IN));
    }
}

void SurveillanceService::slotReceiveIpCameraInfos(QVector<IpCameraInfo> cameraInfos)
{
    if(cameraInfos.size() != getChanelCount())
        return;

    for(int i = 0; i < cameraInfos.size(); i ++)
    {
        setIpCameraInfo(i, cameraInfos[i]);
        setSurveillanceSetting(i, cameraInfos[i]);
    }

    for(int i = 0; i < m_cameraProcessEngines.size(); i ++)
    {
        IpCameraInfo tmpCameraInfo = m_cameraProcessEngines[i]->cameraInfos();
        if(tmpCameraInfo.ipAddress != cameraInfos[i].ipAddress ||
                                tmpCameraInfo.portNum != cameraInfos[i].portNum ||
                                tmpCameraInfo.videoSource != cameraInfos[i].videoSource ||
                                tmpCameraInfo.streamUri != cameraInfos[i].streamUri)
        {
            m_cameraProcessEngines[i]->startProcess(i, cameraInfos[i]);
        }
        else
        {
            m_cameraProcessEngines[i]->setIpCameraInfo(cameraInfos[i]);
            m_cameraProcessEngines[i]->setSurveillanceSetting(cameraInfos[i]);
        }
    }
}

void SurveillanceService::slotReceiveSurveillanceSetting(IpCameraInfo surveillanceSetting, int chanelIndex)
{
    if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
    {
        setSurveillanceSetting(chanelIndex, surveillanceSetting);
        m_cameraProcessEngines[chanelIndex]->setSurveillanceSetting(surveillanceSetting);
    }
}

void SurveillanceService::slotReceiveIpCameraInfo(IpCameraInfo ipCameraInfo, int chanelIndex)
{
    if(chanelIndex >= 0 && chanelIndex < m_cameraProcessEngines.size())
    {
        setIpCameraInfo(chanelIndex, ipCameraInfo);

        IpCameraInfo tmpCameraInfo = m_cameraProcessEngines[chanelIndex]->cameraInfos();
        if(tmpCameraInfo.ipAddress != ipCameraInfo.ipAddress ||
                                tmpCameraInfo.portNum != ipCameraInfo.portNum ||
                                tmpCameraInfo.videoSource != ipCameraInfo.videoSource ||
                                tmpCameraInfo.streamUri != ipCameraInfo.streamUri)
        {
            tmpCameraInfo.ipAddress = ipCameraInfo.ipAddress;
            tmpCameraInfo.portNum = ipCameraInfo.portNum;
            tmpCameraInfo.videoSource = ipCameraInfo.videoSource;
            tmpCameraInfo.streamUri = ipCameraInfo.streamUri;

            m_cameraProcessEngines[chanelIndex]->startProcess(chanelIndex, tmpCameraInfo);
        }
    }
}

void SurveillanceService::slotAuthenticateingFinished(QObject* obj)
{
    for(int i = 0; i < m_authenticatingSockets.size(); i ++)
    {
        if(m_authenticatingSockets[i] == obj)
        {
            m_authenticatingSockets[i]->wait();
            delete m_authenticatingSockets[i];
            m_authenticatingSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotFaceSendingSocketError(QObject* obj)
{
    for(int i = 0; i < m_faceSendingSockets.size(); i ++)
    {
        if(m_faceSendingSockets[i] == obj)
        {
            m_faceSendingSockets[i]->stopSocket();
            delete m_faceSendingSockets[i];
            m_faceSendingSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotFaceImageSendingSocketError(QObject* obj)
{
    for(int i = 0; i < m_faceImageSendingSockets.size(); i ++)
    {
        if(m_faceImageSendingSockets[i] == obj)
        {
            m_faceImageSendingSockets[i]->stopSocket();
            delete m_faceImageSendingSockets[i];
            m_faceImageSendingSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotSceneImageSendFinished(QObject* obj)
{
    for(int i = 0; i < m_sceneImageSendSockets.size(); i ++)
    {
        if(m_sceneImageSendSockets[i] == obj)
        {
            m_sceneImageSendSockets[i]->stopSocket();
            delete m_sceneImageSendSockets[i];
            m_sceneImageSendSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotImageProcessingFinished(QObject* obj)
{
    for(int i = 0; i < m_imageProcessingSockets.size(); i ++)
    {
        if(m_imageProcessingSockets[i] == obj)
        {
            m_imageProcessingSockets[i]->stopSocket();
            delete m_imageProcessingSockets[i];
            m_imageProcessingSockets.remove(i);
            break;
        }
    }
}


void SurveillanceService::slotLogChanged(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults, QVector<LOG_ITEM> logItem, int chanelIndex)
{
#define MAX_SAVE_SIZE 100
    if(m_logSaveThread.size() > MAX_SAVE_SIZE)
        return;

    LogSaveThread* logSaveThread = new LogSaveThread;
    m_logSaveThread.append(logSaveThread);

    connect(logSaveThread, SIGNAL(saveFinished(QObject*)), this, SLOT(slotLogSaveFinished(QObject*)));
    connect(logSaveThread, SIGNAL(savedLogInfo(QDate)), m_logCheckThread, SLOT(slotLogSaved(QDate)));

    logSaveThread->startSaveLog(blackRecogResults, logItem, chanelIndex);
}

void SurveillanceService::slotLogSaveFinished(QObject* obj)
{
    for(int i = 0; i < m_logSaveThread.size(); i ++)
    {
        if(m_logSaveThread[i] == obj)
        {
            m_logSaveThread[i]->wait();
            delete m_logSaveThread[i];
            m_logSaveThread.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotReceiveBlackPersonFinished(QObject* obj)
{
    for(int i = 0; i < m_oneBlackPersonReceiveSockets.size(); i ++)
    {
        if(m_oneBlackPersonReceiveSockets[i] == obj)
        {
            m_oneBlackPersonReceiveSockets[i]->stopSocket();
            delete m_oneBlackPersonReceiveSockets[i];
            m_oneBlackPersonReceiveSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotBlackInfoSendingFinished(QObject* obj)
{
    for(int i = 0; i < m_blackInfoSendingSockets.size(); i ++)
    {
        if(m_blackInfoSendingSockets[i] == obj)
        {
            m_blackInfoSendingSockets[i]->stopSocket();
            delete m_blackInfoSendingSockets[i];
            m_blackInfoSendingSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotReceiveDeleteBlackInfoFinished(QObject* obj)
{
    for(int i = 0; i < m_blackPersonDeleteSockets.size(); i ++)
    {
        if(m_blackPersonDeleteSockets[i] == obj)
        {
            m_blackPersonDeleteSockets[i]->stopSocket();
            delete m_blackPersonDeleteSockets[i];
            m_blackPersonDeleteSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotBlackRecogResultSendSocketError(QObject* obj)
{
    for(int i = 0; i < m_blackRecogResultSendingSockets.size(); i ++)
    {
        if(m_blackRecogResultSendingSockets[i] == obj)
        {
            m_blackRecogResultSendingSockets[i]->stopSocket();
            delete m_blackRecogResultSendingSockets[i];
            m_blackRecogResultSendingSockets.remove(i);
            break;
        }
    }
}


void SurveillanceService::slotReceiveEditBlackInfoFinished(QObject* obj)
{
    for(int i = 0; i < m_oneBlackPersonEditSockets.size(); i ++)
    {
        if(m_oneBlackPersonEditSockets[i] == obj)
        {
            m_oneBlackPersonEditSockets[i]->stopSocket();
            delete m_oneBlackPersonEditSockets[i];
            m_oneBlackPersonEditSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotSearchLogSendingSocketFinished(QObject* obj)
{
    for(int i = 0; i < m_searchLogSendingSockets.size(); i ++)
    {
        if(m_searchLogSendingSockets[i] == obj)
        {
            m_searchLogSendingSockets[i]->stopSocket();
            delete m_searchLogSendingSockets[i];
            m_searchLogSendingSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotSearchBlackSendingSocketFinished(QObject* obj)
{
    for(int i = 0; i < m_searchBlckSendingSockets.size(); i ++)
    {
        if(m_searchBlckSendingSockets[i] == obj)
        {
            m_searchBlckSendingSockets[i]->stopSocket();
            delete m_searchBlckSendingSockets[i];
            m_searchBlckSendingSockets.remove(i);
            break;
        }
    }
}

void SurveillanceService::slotSearchLastBlackSendingSocketFinished(QObject* obj)
{
    for(int i = 0; i < m_searchLastBlckSendingSockets.size(); i ++)
    {
        if(m_searchLastBlckSendingSockets[i] == obj)
        {
            m_searchLastBlckSendingSockets[i]->stopSocket();
            delete m_searchLastBlckSendingSockets[i];
            m_searchLastBlckSendingSockets.remove(i);
            break;
        }
    }
}


void SurveillanceService::slotLogOut(QString logOutStr)
{
    emit logOut(logOutStr);
}

