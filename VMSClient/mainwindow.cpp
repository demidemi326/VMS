#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mediaitem.h"
#include "facerequestsocket.h"
#include "clientbase.h"
#include "logresultreceivesocket.h"
#include "serverdlg.h"
#include "stringtable.h"
#include "serverinfosocket.h"
#include "ipcamerasettingdlg.h"
#include "monitoringareadlg.h"
#include "areaselectdlg.h"
#include "ipcameraonesettingdlg.h"
#include "logviewitem.h"
#include "logselectdlg.h"
#include "blackinforeceivesocket.h"
#include "gendereditor.h"
#include "editblackinfodlg.h"
#include "deleteblacklistsocket.h"
#include "addblacklistfromcaptureddlg.h"
#include "blackrecogresultreceivesocket.h"
#include "blackenrollbatchdlg.h"
#include "addblackinfosocket.h"
#include "editoneblacklistsocket.h"
#include "monitoringserveillancesettingdlg.h"
#include "monitoringipcamerasettngdlg.h"
#include "searchlogsocket.h"
#include "blackfaceadddlg.h"
#include "searchblacksocket.h"
#include "cachelogviewitem.h"
#include "warningplayer.h"
#include "aboutdlg.h"
#include "searchlastblacksocket.h"
#include "selectchaneldlg.h"
#include "newpass.h"
#include "modifypass.h"
#include "removepass.h"
#include "resultalarmdlg.h"
#include "ipcamera.h"
#include "camerasurfaceglitem.h"
#include "camerasurfaceglview.h"
#include "camerasurfaceview.h"

#include <QtWidgets>

int g_fontId = -1;

#define LAST_LOG_DIFF 10

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_deletBlackListSocket = NULL;
    m_blackInfoReceiveSocket = NULL;
    m_addBlackInfoSocket = NULL;
    m_editBlackInfoSocket = NULL;
    m_searchLogSocket = NULL;
    m_searchBlackSocket = NULL;
    m_searchLastBlackSocket = NULL;
    m_cameraViewType = CAMERA_VIEW_1_1;

    m_selectedServerIndexOfEditBlack = -1;
    m_selectedChanelIndexOfEditBlack = -1;
    m_searchLogCount = 0;
    m_searchCurLogPageIndex = 0;
    m_searchAllLogPageCount = 0;
    m_firstStart = 0;
    m_blackAlarmId = 0;


    m_searchBlackCount = 0;
    m_searchCurBlackPageIndex = 0;
    m_searchAllBlackPageCount = 0;

    m_timerID = startTimer(1000 * 60);

    setupActions();
    retranslateUI();
    setupSockets();
    loadSetting();
    loadLog();

    IpCamera::init();

    setupCameraViews();
    setupLogViewItems();

    refreshMonitoringPageArea();
    refreshMonitoringPageBlackResult();
    refreshAreaPageArea();
    refreshServers();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshSearchLogFace();

    slotViewMonitoringPage();
    refreshLogNavigation();
    refreshBlackNavigation();
    refreshActions();

    loadViewSetting();

//    resize(qApp->desktop()->availableGeometry().width() - 50, qApp->desktop()->availableGeometry().height() - 50);
//    move(0, 0);
}

MainWindow::~MainWindow()
{
    saveViewSetting();

    m_progressDialog->close();
    stopAddBlackInfoSocket();
    stopEditBlackInfoSocket();
    stopSearchLogSockets();
    stopSearchBlackSockets();
    stopSearchLastBlackSockets();

    initCameraViewItem();
    stopBlackInfoReceive();

    saveSetting();
    saveLog();


    WSACleanup();

    delete ui;
}

void MainWindow::loadSetting()
{
    QFile dataFile("./_setting_video.db");

    bool opened = dataFile.open(QIODevice::ReadOnly);
    if(opened == false)
        return;

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    int serverInfoCount = 0;

    dataStream >> serverInfoCount;
    for(int i = 0; i < serverInfoCount; i ++)
    {
        ServerInfo serverInfo;
        dataStream >> serverInfo.serverName;
        dataStream >> serverInfo.ipAddress;
        dataStream >> serverInfo.port;
        dataStream >> serverInfo.userName;
        dataStream >> serverInfo.password;
        dataStream >> serverInfo.serverTypeStr;
        dataStream >> serverInfo.serverUID;

        int chanelCount = 0;
        dataStream >> chanelCount;

        for(int i = 0; i < chanelCount; i ++)
        {
            IpCameraInfo ipCameraInfo;

            dataStream >> ipCameraInfo.ipAddress;
            dataStream >> ipCameraInfo.portNum;
            dataStream >> ipCameraInfo.videoSource;
            dataStream >> ipCameraInfo.streamuri;

            ipCameraInfo.blackInfoSize = 0;
            serverInfo.ipCameraInfos.append(ipCameraInfo);
        }

        addServerSocket(serverInfo);
    }

    int monitoringAreaCount = 0;
    dataStream >> monitoringAreaCount;

    QVector<int> serverIndexs;
    QVector<int> chanelIndexs;
    QVector<QString> areaNames;
    for(int i = 0; i < monitoringAreaCount; i ++)
    {
        MonitoringAreaInfo monitoringAreaInfo;
        dataStream >> monitoringAreaInfo.areaName;
        dataStream >> monitoringAreaInfo.serverIndexs;
        dataStream >> monitoringAreaInfo.chanelIndexs;

        serverIndexs << monitoringAreaInfo.serverIndexs;
        chanelIndexs << monitoringAreaInfo.serverIndexs;
        for(int j = 0; j < monitoringAreaInfo.serverIndexs.size(); j ++)
            areaNames << monitoringAreaInfo.areaName;

        m_areaInfos.append(monitoringAreaInfo);

        for(int j = 0; j < monitoringAreaInfo.serverIndexs.size(); j ++)
        {
            addLogFaceReceiveSocket(monitoringAreaInfo.serverIndexs[j], monitoringAreaInfo.chanelIndexs[j]);
            addBlackRecogResultReceiveSocket(monitoringAreaInfo.serverIndexs[j], monitoringAreaInfo.chanelIndexs[j]);
        }
    }
    dataFile.close();
}

void MainWindow::saveSetting()
{
    QFile dataFile("./_setting_video.db");

    bool opened = dataFile.open(QIODevice::WriteOnly);
    if(opened == false)
        return;

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    dataStream << m_serverInfoSockets.size();
    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        ServerInfo serverInfo = m_serverInfoSockets[i]->serverInfo();
        dataStream << serverInfo.serverName;
        dataStream << serverInfo.ipAddress;
        dataStream << serverInfo.port;
        dataStream << serverInfo.userName;
        dataStream << serverInfo.password;
        dataStream << serverInfo.serverTypeStr;
        dataStream << serverInfo.serverUID;
        dataStream << serverInfo.ipCameraInfos.size();

        for(int j = 0; j < serverInfo.ipCameraInfos.size(); j ++)
        {
            dataStream << serverInfo.ipCameraInfos[j].ipAddress;
            dataStream << serverInfo.ipCameraInfos[j].portNum;
            dataStream << serverInfo.ipCameraInfos[j].videoSource;
            dataStream << serverInfo.ipCameraInfos[j].streamuri;
        }
    }

    dataStream << m_areaInfos.size();
    for(int i = 0; i < m_areaInfos.size(); i ++)
    {
        dataStream << m_areaInfos[i].areaName;
        dataStream << m_areaInfos[i].serverIndexs;
        dataStream << m_areaInfos[i].chanelIndexs;
    }

    dataStream << m_faceImageInfos.size();
    for(int i = 0; i < m_faceImageInfos.size(); i ++)
    {
        dataStream << m_faceImageInfos[i].serverUID;
        dataStream << m_faceImageInfos[i].chanelIndex;
        dataStream << m_faceImageInfos[i].areaName;
        dataStream << m_faceImageInfos[i].logUID;
        dataStream << m_faceImageInfos[i].dateTime;
        dataStream << m_faceImageInfos[i].faceImage;

        for(int j = 0; j < 3; j ++)
        {
            dataStream << m_faceImageInfos[i].candidateFace[j];
            dataStream << m_faceImageInfos[i].candidateDist[j];
            dataStream << m_faceImageInfos[i].candidateType[j];
        }

    }
    dataFile.close();
}

void MainWindow::loadLog()
{
    QFile dataFile("./_logdata_video.db");

    bool opened = dataFile.open(QIODevice::ReadOnly);
    if(opened == false)
        return;

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    int faceLogCount = 0;
    dataStream >> faceLogCount;

    for(int i = 0; i < faceLogCount; i ++)
    {
        LOG_RESULT logResult;
        dataStream >> logResult.serverUID;
        dataStream >> logResult.chanelIndex;
        dataStream >> logResult.areaName;
        dataStream >> logResult.logUID;
        dataStream >> logResult.dateTime;
        dataStream >> logResult.faceImage;

        for(int j = 0; j < 3; j ++)
        {
            dataStream >> logResult.candidateFace[j];
            dataStream >> logResult.candidateDist[j];
            dataStream >> logResult.candidateType[j];
        }

        m_faceImageInfos.append(logResult);
    }

    int viewLogCount = 0;
    dataStream >> viewLogCount;
    ui->spinMonitoringPageLogCount->setValue(viewLogCount);


    int blackRecogResultCount = 0;
    dataStream >> blackRecogResultCount;

    for(int i = 0; i < blackRecogResultCount; i ++)
    {
        int candiateCount = 0;
        dataStream >> candiateCount;

        QVector<BLACK_RECOG_RESULT> candidateResult;
        for(int j = 0; j < candiateCount; j ++)
        {
            BLACK_RECOG_RESULT blackRecogResult;
            dataStream >> blackRecogResult.name;
            dataStream >> blackRecogResult.gender;
            dataStream >> blackRecogResult.birthday;
            dataStream >> blackRecogResult.address;
            dataStream >> blackRecogResult.description;
            dataStream >> blackRecogResult.personType;
            dataStream >> blackRecogResult.galleryFaceData;
            dataStream >> blackRecogResult.probeFaceData;
            dataStream >> blackRecogResult.similiarity;
            dataStream >> blackRecogResult.logUID;
            dataStream >> blackRecogResult.dateTime;
            dataStream >> blackRecogResult.serverUID;
            dataStream >> blackRecogResult.chanelIndex;
            dataStream >> blackRecogResult.areaName;

            candidateResult.append(blackRecogResult);
        }

        m_blackRecogResults.append(candidateResult);
    }


    dataFile.close();
}

void MainWindow::saveLog()
{
    QFile dataFile("./_logdata_video.db");

    bool opened = dataFile.open(QIODevice::WriteOnly);
    if(opened == false)
        return;

    qDebug() << "save log" << m_faceImageInfos.size() << m_blackRecogResults.size();

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    dataStream << m_faceImageInfos.size();
    for(int i = 0; i < m_faceImageInfos.size(); i ++)
    {
        dataStream << m_faceImageInfos[i].serverUID;
        dataStream << m_faceImageInfos[i].chanelIndex;
        dataStream << m_faceImageInfos[i].areaName;
        dataStream << m_faceImageInfos[i].logUID;
        dataStream << m_faceImageInfos[i].dateTime;
        dataStream << m_faceImageInfos[i].faceImage;

        for(int j = 0; j < 3; j ++)
        {
            dataStream << m_faceImageInfos[i].candidateFace[j];
            dataStream << m_faceImageInfos[i].candidateDist[j];
            dataStream << m_faceImageInfos[i].candidateType[j];
        }

    }

    dataStream << ui->spinMonitoringPageLogCount->value();

    dataStream << m_blackRecogResults.size();
    for(int i = 0; i < m_blackRecogResults.size(); i ++)
    {
        dataStream << m_blackRecogResults[i].size();

        for(int j = 0; j < m_blackRecogResults[i].size(); j ++)
        {
            dataStream << m_blackRecogResults[i][j].name;
            dataStream << m_blackRecogResults[i][j].gender;
            dataStream << m_blackRecogResults[i][j].birthday;
            dataStream << m_blackRecogResults[i][j].address;
            dataStream << m_blackRecogResults[i][j].description;
            dataStream << m_blackRecogResults[i][j].personType;
            dataStream << m_blackRecogResults[i][j].galleryFaceData;
            dataStream << m_blackRecogResults[i][j].probeFaceData;
            dataStream << m_blackRecogResults[i][j].similiarity;
            dataStream << m_blackRecogResults[i][j].logUID;
            dataStream << m_blackRecogResults[i][j].dateTime;
            dataStream << m_blackRecogResults[i][j].serverUID;
            dataStream << m_blackRecogResults[i][j].chanelIndex;
            dataStream << m_blackRecogResults[i][j].areaName;
        }
    }

    dataFile.close();
}

void MainWindow::loadViewSetting()
{
    QFile dataFile("./_viewsetting_video.db");

    bool opened = dataFile.open(QIODevice::ReadOnly);
    if(opened == false)
        return;

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    int selectedCameraViewIndex = 0;
    int isMaximum = 0;
    dataStream >> m_cameraViewType;
    dataStream >> selectedCameraViewIndex;
    dataStream >> isMaximum;

    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        int hasMonitoring = 0;
        dataStream >> hasMonitoring;
        if(hasMonitoring == 1)
        {
            int serverUID, chanelIndex;
            dataStream >> serverUID;
            dataStream >> chanelIndex;

            int serverIndex = getServerIndexFromUID(m_serverInfoSockets, serverUID);
            QString areaName= getAreaNameFromUID(m_areaInfos, m_serverInfoSockets, serverUID, chanelIndex);

            if(serverIndex >= 0 && serverIndex < m_serverInfoSockets.size() && !areaName.isEmpty())
                m_cameraViewItems[i]->startMonitoring(m_serverInfoSockets[serverIndex], chanelIndex, areaName, i);
        }
    }

    dataFile.close();

    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        if(i == selectedCameraViewIndex)
        {
            m_cameraViewItems[i]->setSelected(true);
            if(isMaximum)
                m_cameraViewItems[i]->setMaximum(1);
        }
        else
            m_cameraViewItems[i]->setSelected(false);

    }

    refreshCameraBar();
}


void MainWindow::saveViewSetting()
{
    int selectedCameraViewIndex = 0;
    int isMaximum = 0;
    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        if(m_cameraViewItems[i]->isSelected())
        {
            selectedCameraViewIndex = i;
            isMaximum = m_cameraViewItems[i]->isMaximum();
        }
    }

    QFile dataFile("./_viewsetting_video.db");

    bool opened = dataFile.open(QIODevice::WriteOnly);
    if(opened == false)
        return;

    QDataStream dataStream(&dataFile);
    dataStream.setVersion(QDataStream::Qt_5_0);

    dataStream << m_cameraViewType;
    dataStream << selectedCameraViewIndex;
    dataStream << isMaximum;

    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        int hasMonitoring = 0;
        if(m_cameraViewItems[i]->serverInfoSocket())
        {
            hasMonitoring = 1;
            dataStream << hasMonitoring;

            int serverUID = m_cameraViewItems[i]->serverInfoSocket()->serverInfo().serverUID;
            int chanelIndex = m_cameraViewItems[i]->chanelIndex();

            dataStream << serverUID;
            dataStream << chanelIndex;
        }
        else
            dataStream << hasMonitoring;
    }

    dataFile.close();
}


void MainWindow::setupActions()
{
    m_toolbarMonitoringPageCamera = new QToolBar;
    m_toolbarMonitoringPageCamera->addAction(ui->actionCamera1_1);
    m_toolbarMonitoringPageCamera->addAction(ui->actionCamera2_2);
    m_toolbarMonitoringPageCamera->addAction(ui->actionCamera3_3);
    m_toolbarMonitoringPageCamera->addAction(ui->actionCamera4_4);
    m_toolbarMonitoringPageCamera->addSeparator();
    m_toolbarMonitoringPageCamera->addAction(ui->actionCapture);

    m_toolbarMonitoringPageLog = new QToolBar;
    m_toolbarMonitoringPageLog->addWidget(ui->lblMonitoringPageCurLogCount);
    m_toolbarMonitoringPageLog->addWidget(ui->spinMonitoringPageLogCount);

    QWidget* widCamera = new QWidget;
    QVBoxLayout* cameraLayout = new QVBoxLayout;
    cameraLayout->addWidget(m_toolbarMonitoringPageCamera);
    cameraLayout->addWidget(ui->viewMonitoringPageCamera);
    cameraLayout->setMargin(0);
    cameraLayout->setSpacing(0);
    widCamera->setLayout(cameraLayout);

    QWidget* widLog = new QWidget;
    QVBoxLayout* logLayout = new QVBoxLayout;
    logLayout->addWidget(m_toolbarMonitoringPageLog);
    logLayout->addWidget(ui->viewMonitoringPageLogResult);
    logLayout->setMargin(0);
    logLayout->setSpacing(10);
    widLog->setLayout(logLayout);

    QVBoxLayout* blackListBox = new QVBoxLayout;
    blackListBox->setMargin(0);
    blackListBox->setSpacing(5);
    blackListBox->addWidget(ui->viewMonitoringPageArea);
    blackListBox->addWidget(ui->widBlack);

    QWidget* widBlackView = new QWidget;
    widBlackView->setLayout(blackListBox);


    QSplitter* monitoringHSplitter = new QSplitter(Qt::Horizontal);
    monitoringHSplitter->addWidget(widCamera);
    monitoringHSplitter->addWidget(widLog);
    ui->viewMonitoringPageLogResult->resize(300, ui->viewMonitoringPageLogResult->height());

    monitoringHSplitter->setStretchFactor(0, 100);
    monitoringHSplitter->setStretchFactor(1, 1);

    QSplitter* monitoringVSplitter = new QSplitter(Qt::Vertical);
    monitoringVSplitter->addWidget(monitoringHSplitter);
    monitoringVSplitter->addWidget(ui->viewMonitoringPageBlackResult);
    monitoringVSplitter->setStretchFactor(0, 100);
    monitoringVSplitter->setStretchFactor(1, 1);

    QSplitter* monitoringMainSplitter = new QSplitter(ui->pageMonitoring);
    monitoringMainSplitter->addWidget(widBlackView);
    monitoringMainSplitter->addWidget(monitoringVSplitter);
    monitoringMainSplitter->setChildrenCollapsible(true);
    monitoringMainSplitter->setStretchFactor(0, 1);
    monitoringMainSplitter->setStretchFactor(1, 100);


    QHBoxLayout* monitoringPageLayout = new QHBoxLayout;
    monitoringPageLayout->addWidget(monitoringMainSplitter);
    ui->pageMonitoring->setLayout(monitoringPageLayout);

    QToolButton* btnMonitering = new QToolButton;
    btnMonitering->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnMonitering->setDefaultAction(ui->actionMonitoringPage);
    btnMonitering->setFixedWidth(80);
    ui->mainToolBar->addWidget(btnMonitering);

    QToolButton* btnMachines = new QToolButton;
    btnMachines->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnMachines->setDefaultAction(ui->actionServer);
    btnMachines->setFixedWidth(80);
    ui->mainToolBar->addWidget(btnMachines);

    QToolButton* btnMonitoringArea = new QToolButton;
    btnMonitoringArea->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnMonitoringArea->setDefaultAction(ui->actionMonitoringArea);
    btnMonitoringArea->setFixedWidth(80);
    ui->mainToolBar->addWidget(btnMonitoringArea);

    QToolButton* btnBlackList = new QToolButton;
    btnBlackList->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnBlackList->setDefaultAction(ui->actionBlackList);
    btnBlackList->setFixedWidth(80);
    ui->mainToolBar->addWidget(btnBlackList);

    QToolButton* btnSearhPage = new QToolButton;
    btnSearhPage->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnSearhPage->setDefaultAction(ui->actionSearchPage);
    btnSearhPage->setFixedWidth(80);
    ui->mainToolBar->addWidget(btnSearhPage);
    ui->mainToolBar->addSeparator();

    m_menuPassword = new QMenu;
    m_menuPassword->addAction(ui->actionNewPassword);
    m_menuPassword->addAction(ui->actionChangePassword);
    m_menuPassword->addAction(ui->actionRemovePassword);

    m_menuPassword->menuAction()->setIcon(QIcon(":/images/security.png"));

    QToolButton* btnPassword = new QToolButton;
    btnPassword->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnPassword->setDefaultAction(m_menuPassword->menuAction());
    btnPassword->setFixedWidth(80);
    ui->mainToolBar->addWidget(btnPassword);

    QToolButton* btnAboutPage = new QToolButton;
    btnAboutPage->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnAboutPage->setDefaultAction(ui->actionAbout);
    btnAboutPage->setFixedWidth(80);
    ui->mainToolBar->addWidget(btnAboutPage);

    m_monitoringPageAreaModel = new QStandardItemModel(0, 1);
    m_monitoringPageAreaModel->setHeaderData(0, Qt::Horizontal, StringTable::Str_Monitoring_Area);

    m_monitoringPageAreaSelectionModel = new QItemSelectionModel(m_monitoringPageAreaModel);

    ui->viewMonitoringPageArea->setModel(m_monitoringPageAreaModel);
    ui->viewMonitoringPageArea->setSelectionModel(m_monitoringPageAreaSelectionModel);
    ui->viewMonitoringPageArea->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewMonitoringPageArea->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewMonitoringPageArea->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewMonitoringPageArea->setAlternatingRowColors(true);

    m_monitoringAreaContextMenu = new QMenu;
//    m_monitoringAreaContextMenu->addAction(ui->actionCameraSetting);
    m_monitoringAreaContextMenu->addAction(ui->actionSurveillanceSetting);

    m_sceneMonitoringPageCamera = new QGraphicsScene;
//    ui->viewMonitoringPageCamera->setScene(m_sceneMonitoringPageCamera);
//    ui->viewMonitoringPageCamera->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//    ui->viewMonitoringPageCamera->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    ui->viewMonitoringPageCamera->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_sceneMonitoringPageLog = new QGraphicsScene;
    ui->viewMonitoringPageLogResult->setScene(m_sceneMonitoringPageLog);
    ui->viewMonitoringPageLogResult->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_monitoringPageBlackResultModel = new QStandardItemModel(0, 9);
    m_monitoringPageBlackResultModel->setHeaderData(0, Qt::Horizontal, StringTable::Str_Name);
    m_monitoringPageBlackResultModel->setHeaderData(1, Qt::Horizontal, StringTable::Str_Group);
    m_monitoringPageBlackResultModel->setHeaderData(2, Qt::Horizontal, StringTable::Str_Monitoring_Area);
    m_monitoringPageBlackResultModel->setHeaderData(3, Qt::Horizontal, StringTable::Str_Time);
    m_monitoringPageBlackResultModel->setHeaderData(4, Qt::Horizontal, StringTable::Str_Similiarity);
    m_monitoringPageBlackResultModel->setHeaderData(5, Qt::Horizontal, StringTable::Str_Gender);
    m_monitoringPageBlackResultModel->setHeaderData(6, Qt::Horizontal, StringTable::Str_Birthday);
    m_monitoringPageBlackResultModel->setHeaderData(7, Qt::Horizontal, StringTable::Str_Address);
    m_monitoringPageBlackResultModel->setHeaderData(8, Qt::Horizontal, StringTable::Str_Description);

    m_monitoringPageBlackResultSelectionModel = new QItemSelectionModel(m_monitoringPageBlackResultModel);

    ui->viewMonitoringPageBlackResult->setModel(m_monitoringPageBlackResultModel);
    ui->viewMonitoringPageBlackResult->setSelectionModel(m_monitoringPageBlackResultSelectionModel);
    ui->viewMonitoringPageBlackResult->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewMonitoringPageBlackResult->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewMonitoringPageBlackResult->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewMonitoringPageBlackResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewMonitoringPageBlackResult->setAlternatingRowColors(true);
    ui->viewMonitoringPageBlackResult->setColumnWidth(3, 200);

    m_toolbarServerPageServer = new QToolBar;
    QHBoxLayout* logToolBarLayout = new QHBoxLayout;
    logToolBarLayout->addWidget(m_toolbarServerPageServer);
    logToolBarLayout->setMargin(0);
    logToolBarLayout->setSpacing(0);
    ui->widgetMachineToolBar->setLayout(logToolBarLayout);

    m_toolbarServerPageServer->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbarServerPageServer->addAction(ui->actionAddServer);
    m_toolbarServerPageServer->addAction(ui->actionEditServer);
    m_toolbarServerPageServer->addAction(ui->actionDeleteServer);
    m_toolbarServerPageServer->addSeparator();
    m_toolbarServerPageServer->addAction(ui->actionEditIPCameras);

    m_modelServerPageServer = new QStandardItemModel(0, 5);
    m_modelServerPageServer->setHeaderData(0, Qt::Horizontal, StringTable::Str_Machine_Name);
    m_modelServerPageServer->setHeaderData(1, Qt::Horizontal, StringTable::Str_IP_Address);
    m_modelServerPageServer->setHeaderData(2, Qt::Horizontal, StringTable::Str_Chanel_Count);
    m_modelServerPageServer->setHeaderData(3, Qt::Horizontal, StringTable::Str_Machine_Type);
    m_modelServerPageServer->setHeaderData(4, Qt::Horizontal, StringTable::Str_Status);

    m_selectionModelServerPageServer = new QItemSelectionModel(m_modelServerPageServer);

    ui->viewServerPageServer->setModel(m_modelServerPageServer);
    ui->viewServerPageServer->setSelectionModel(m_selectionModelServerPageServer);
    ui->viewServerPageServer->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewServerPageServer->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewServerPageServer->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewServerPageServer->setGridStyle(Qt::NoPen);
    ui->viewServerPageServer->setAlternatingRowColors(true);
    ui->viewServerPageServer->verticalHeader()->setDefaultSectionSize(24);
    ui->viewServerPageServer->verticalHeader()->setHighlightSections(false);
    ui->viewServerPageServer->horizontalHeader()->setHighlightSections(false);
    ui->viewServerPageServer->setColumnWidth(3, 200);

    m_toolbarAreaPageArea = new QToolBar;
    QHBoxLayout* areaToolBarLayout = new QHBoxLayout;
    areaToolBarLayout->addWidget(m_toolbarAreaPageArea);
    areaToolBarLayout->setMargin(0);
    areaToolBarLayout->setSpacing(0);
    ui->widgetMonitoringAreaToolBar->setLayout(areaToolBarLayout);

    m_toolbarAreaPageArea->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbarAreaPageArea->addAction(ui->actionAddArea);
    m_toolbarAreaPageArea->addAction(ui->actionEditArea);
    m_toolbarAreaPageArea->addAction(ui->actionDeleteArea);

    m_modelAreaPageArea = new QStandardItemModel(0, 1);
    m_modelAreaPageArea->setHeaderData(0, Qt::Horizontal, StringTable::Str_Monitoring_Area);

    m_selectionModelAreaPageArea = new QItemSelectionModel(m_modelAreaPageArea);

    ui->viewAreaPageArea->setModel(m_modelAreaPageArea);
    ui->viewAreaPageArea->setSelectionModel(m_selectionModelAreaPageArea);
    ui->viewAreaPageArea->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewAreaPageArea->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewAreaPageArea->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewAreaPageArea->setAlternatingRowColors(true);

    m_modelAreaPageServer = new QStandardItemModel(0, 1);
    m_modelAreaPageServer->setHeaderData(0, Qt::Horizontal, StringTable::Str_Server);

    m_selectionModelAreaPageServer = new QItemSelectionModel(m_modelAreaPageServer);

    ui->viewAreaPageServer->setModel(m_modelAreaPageServer);
    ui->viewAreaPageServer->setSelectionModel(m_selectionModelAreaPageServer);
    ui->viewAreaPageServer->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewAreaPageServer->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewAreaPageServer->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewAreaPageServer->setAlternatingRowColors(true);

    QWidget* widgetEditBlackList = new QWidget;
    QWidget* widgetSubEditBlackList = new QWidget;

    QVBoxLayout* widgetRightEditBlackListLayout = new QVBoxLayout;
    widgetRightEditBlackListLayout->addWidget(ui->viewEditBlackList);
    widgetRightEditBlackListLayout->addWidget(ui->progressBarBlack);
    widgetRightEditBlackListLayout->setMargin(0);
    widgetRightEditBlackListLayout->setSpacing(6);

    QHBoxLayout* widgetSubEditBlackListLayout = new QHBoxLayout;
    widgetSubEditBlackListLayout->setMargin(0);
    widgetSubEditBlackListLayout->setSpacing(0);
    widgetSubEditBlackListLayout->addLayout(widgetRightEditBlackListLayout);
    widgetSubEditBlackListLayout->addWidget(ui->lblBlackInfoFace);
    ui->progressBarBlack->hide();

    widgetSubEditBlackList->setLayout(widgetSubEditBlackListLayout);

    m_toolbarBlackPage = new QToolBar;
    QVBoxLayout* editBlackListRightLayout = new QVBoxLayout;
    editBlackListRightLayout->addWidget(m_toolbarBlackPage);
    editBlackListRightLayout->addWidget(widgetSubEditBlackList);
    editBlackListRightLayout->setMargin(0);
    editBlackListRightLayout->setSpacing(0);
    widgetEditBlackList->setLayout(editBlackListRightLayout);

    m_toolbarBlackPage->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbarBlackPage->addAction(ui->actionAddBlackFromFile);
    m_toolbarBlackPage->addAction(ui->actionAddBlackFromBatch);
    m_toolbarBlackPage->addSeparator();
    m_toolbarBlackPage->addAction(ui->actionEditBlack);
    m_toolbarBlackPage->addAction(ui->actionDeleteBlack);
    m_toolbarBlackPage->addSeparator();
    m_toolbarBlackPage->addAction(ui->actionSendToBlack);
    m_toolbarBlackPage->addAction(ui->actionExportBlack);
    m_toolbarBlackPage->addSeparator();
    m_toolbarBlackPage->addAction(ui->actionImportBlack);

    QSplitter* blackSplitter = new QSplitter(Qt::Horizontal);
    blackSplitter->addWidget(ui->viewEditBlackArea);
    blackSplitter->addWidget(widgetEditBlackList);
    blackSplitter->setStretchFactor(0, 1);
    blackSplitter->setStretchFactor(1, 100);

    QHBoxLayout* editBlackListLayout = new QHBoxLayout;
    editBlackListLayout->addWidget(blackSplitter);
    ui->pageBlackList->setLayout(editBlackListLayout);

    m_modelBlackPageArea = new QStandardItemModel(0, 1);
    m_modelBlackPageArea->setHeaderData(0, Qt::Horizontal, StringTable::Str_Monitoring_Area);

    m_selectionModelBlackPageArea = new QItemSelectionModel(m_modelBlackPageArea);

    ui->viewEditBlackArea->setModel(m_modelBlackPageArea);
    ui->viewEditBlackArea->setSelectionModel(m_selectionModelBlackPageArea);
    ui->viewEditBlackArea->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewEditBlackArea->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewEditBlackArea->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewEditBlackArea->setAlternatingRowColors(true);

    m_modelBlackPageBlack = new QStandardItemModel(0, 6);
    m_modelBlackPageBlack->setHeaderData(0, Qt::Horizontal, StringTable::Str_Name);
    m_modelBlackPageBlack->setHeaderData(1, Qt::Horizontal, StringTable::Str_Group);
    m_modelBlackPageBlack->setHeaderData(2, Qt::Horizontal, StringTable::Str_Gender);
    m_modelBlackPageBlack->setHeaderData(3, Qt::Horizontal, StringTable::Str_Birthday);
    m_modelBlackPageBlack->setHeaderData(4, Qt::Horizontal, StringTable::Str_Address);
    m_modelBlackPageBlack->setHeaderData(5, Qt::Horizontal, StringTable::Str_Description);

    m_selectionModelBlackPageBlack = new QItemSelectionModel(m_modelBlackPageBlack);

    ui->viewEditBlackList->setModel(m_modelBlackPageBlack);
    ui->viewEditBlackList->setSelectionModel(m_selectionModelBlackPageBlack);
    ui->viewEditBlackList->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewEditBlackList->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->viewEditBlackList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewEditBlackList->setGridStyle(Qt::NoPen);
    ui->viewEditBlackList->setAlternatingRowColors(true);
    ui->viewEditBlackList->verticalHeader()->setDefaultSectionSize(24);
    ui->viewEditBlackList->verticalHeader()->setHighlightSections(false);
    ui->viewEditBlackList->horizontalHeader()->setHighlightSections(false);
    ui->viewEditBlackList->setColumnWidth(4, 150);
    ui->viewEditBlackList->setColumnWidth(5, 150);

    m_toolbarSearchPage = new QToolBar;
    QVBoxLayout* layoutSearchPageRight = new QVBoxLayout;
    layoutSearchPageRight->addWidget(m_toolbarSearchPage);
    layoutSearchPageRight->addWidget(ui->uiSearchPageManager);
    layoutSearchPageRight->setMargin(10);
    layoutSearchPageRight->setSpacing(10);

    m_toolbarSearchPage->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbarSearchPage->addAction(ui->actionSearchPageLog);
    m_toolbarSearchPage->addAction(ui->actionSearchPageBlack);

    ui->pageSearch->setLayout(layoutSearchPageRight);

    m_modelSearchPageLogArea = new QStandardItemModel(0, 1);
    m_modelSearchPageLogArea->setHeaderData(0, Qt::Horizontal, StringTable::Str_Monitoring_Area);

    m_selectionModelSearchPageLogArea = new QItemSelectionModel(m_modelSearchPageLogArea);

    ui->viewSearchPageLogArea->setModel(m_modelSearchPageLogArea);
    ui->viewSearchPageLogArea->setSelectionModel(m_selectionModelSearchPageLogArea);
    ui->viewSearchPageLogArea->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewSearchPageLogArea->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewSearchPageLogArea->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewSearchPageLogArea->setAlternatingRowColors(true);

    m_modelSearchPageBlackArea = new QStandardItemModel(0, 1);
    m_modelSearchPageBlackArea->setHeaderData(0, Qt::Horizontal, StringTable::Str_Monitoring_Area);

    m_selectionModelSearchPageBlackArea = new QItemSelectionModel(m_modelSearchPageBlackArea);

    ui->viewSearchPageBlackArea->setModel(m_modelSearchPageBlackArea);
    ui->viewSearchPageBlackArea->setSelectionModel(m_selectionModelSearchPageBlackArea);
    ui->viewSearchPageBlackArea->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->viewSearchPageBlackArea->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->viewSearchPageBlackArea->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewSearchPageBlackArea->setAlternatingRowColors(true);

    m_sceneSearchPageLogResult = new QGraphicsScene;
    ui->viewSeachPageLogResult->setScene(m_sceneSearchPageLogResult);
    ui->viewSeachPageLogResult->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->viewSeachPageLogResult->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_modelSearchPageBlackList = new QStandardItemModel(0, 9);
    m_modelSearchPageBlackList->setHeaderData(0, Qt::Horizontal, StringTable::Str_Name);
    m_modelSearchPageBlackList->setHeaderData(1, Qt::Horizontal, StringTable::Str_Group);
    m_modelSearchPageBlackList->setHeaderData(2, Qt::Horizontal, StringTable::Str_Monitoring_Area);
    m_modelSearchPageBlackList->setHeaderData(3, Qt::Horizontal, StringTable::Str_Time);
    m_modelSearchPageBlackList->setHeaderData(4, Qt::Horizontal, StringTable::Str_Similiarity);
    m_modelSearchPageBlackList->setHeaderData(5, Qt::Horizontal, StringTable::Str_Gender);
    m_modelSearchPageBlackList->setHeaderData(6, Qt::Horizontal, StringTable::Str_Birthday);
    m_modelSearchPageBlackList->setHeaderData(7, Qt::Horizontal, StringTable::Str_Address);
    m_modelSearchPageBlackList->setHeaderData(8, Qt::Horizontal, StringTable::Str_Description);

    m_selectionModelSearchPageBlackList = new QItemSelectionModel(m_modelSearchPageBlackList);

    ui->viewSearchBlackList->setModel(m_modelSearchPageBlackList);
    ui->viewSearchBlackList->setSelectionModel(m_selectionModelSearchPageBlackList);
    ui->viewSearchBlackList->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewSearchBlackList->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewSearchBlackList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewSearchBlackList->setGridStyle(Qt::NoPen);
    ui->viewSearchBlackList->setAlternatingRowColors(true);
    ui->viewSearchBlackList->verticalHeader()->setDefaultSectionSize(30);
    ui->viewSearchBlackList->verticalHeader()->setHighlightSections(false);
    ui->viewSearchBlackList->horizontalHeader()->setHighlightSections(false);
    ui->viewSearchBlackList->setColumnWidth(3, 200);

    ui->dateTimeSearchLogStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchLogEnd->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeSearchBlackStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchBlackEnd->setDateTime(QDateTime::currentDateTime());

    ui->progressSearchLog->hide();
    ui->progressSearchBlack->hide();

    ui->viewMonitoringPageCamera->installEventFilter(this);
    ui->viewMonitoringPageLogResult->installEventFilter(this);
    ui->viewSeachPageLogResult->installEventFilter(this);

    QRegExp numRegExp("([0-9])([0-9])([0-9])([0-9])([0-9])([0-9])");
    QValidator *numRegExpValidator = new QRegExpValidator(numRegExp);
    ui->editCurLog->setValidator(numRegExpValidator);
    ui->editCurBlack->setValidator(numRegExpValidator);

    m_resultAlarmDlg = new ResultAlarmDlg;
    m_resultAlarmDlg->hide();

    m_progressDialog = new QProgressDialog(this);
    m_progressDialog->setModal(true);
    m_progressDialog->setCancelButton(NULL);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint);
    m_progressDialog->setAutoClose(false);
    m_progressDialog->setAutoReset(false);

    qRegisterMetaType<QVector<QRect> >("QVector<QRect>");
    qRegisterMetaType<HBITMAP>("HBITMAP");
    qRegisterMetaType<HDC>("HDC");
    qRegisterMetaType<LOG_RESULT>("LOG_RESULT");
    qRegisterMetaType<FRAME_RESULT>("FRAME_RESULT");
    qRegisterMetaType<QVector<FRAME_RESULT> >("QVector<FRAME_RESULT>");
    qRegisterMetaType<QVector<BLACK_PERSON> >("QVector<BLACK_PERSON>");
    qRegisterMetaType<QVector<BLACK_RECOG_RESULT> >("QVector<BLACK_RECOG_RESULT>");
    qRegisterMetaType<YUVFrame>("YUVFrame");

    connect(ui->actionMonitoringPage, SIGNAL(triggered()), this, SLOT(slotViewMonitoringPage()));
    connect(ui->actionServer, SIGNAL(triggered()), this, SLOT(slotViewServerPage()));
    connect(ui->actionMonitoringArea, SIGNAL(triggered()), this, SLOT(slotViewAreaPage()));
    connect(ui->actionBlackList, SIGNAL(triggered()), this, SLOT(slotViewBlackListPage()));
    connect(ui->actionSearchPage, SIGNAL(triggered()), this, SLOT(slotViewSearchPage()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(slotAbout()));

    connect(ui->actionCamera1_1, SIGNAL(triggered()), this, SLOT(slotViewCamera1_1()));
    connect(ui->actionCamera2_2, SIGNAL(triggered()), this, SLOT(slotViewCamera2_2()));
    connect(ui->actionCamera3_3, SIGNAL(triggered()), this, SLOT(slotViewCamera3_3()));
    connect(ui->actionCamera4_4, SIGNAL(triggered()), this, SLOT(slotViewCamera4_4()));
    connect(ui->actionCapture, SIGNAL(triggered()), this, SLOT(slotCapture()));

    connect(ui->actionCameraSetting, SIGNAL(triggered()), this, SLOT(slotMonitoringPageCameraSetting()));
    connect(ui->actionSurveillanceSetting, SIGNAL(triggered()), this, SLOT(slotMonitoringPageSurveillanceSetting()));

    connect(ui->viewMonitoringPageArea, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotMonitoringPageAreaItemDoubleClicked(QModelIndex)));

    ui->viewMonitoringPageArea->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->viewMonitoringPageArea, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotMonitoringPageAreaItemContext(QPoint)));


    connect(ui->spinMonitoringPageLogCount, SIGNAL(valueChanged(int)), this, SLOT(setupLogViewItems()));
    connect(m_monitoringPageBlackResultSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotBlackResultSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->viewMonitoringPageBlackResult, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotBlackResultDoubleClicked(QModelIndex)));

    connect(ui->actionAddServer, SIGNAL(triggered()), this, SLOT(slotAddServer()));
    connect(ui->actionEditServer, SIGNAL(triggered()), this, SLOT(slotEditServer()));
    connect(ui->actionDeleteServer, SIGNAL(triggered()), this, SLOT(slotDeleteServer()));
    connect(ui->actionEditIPCameras, SIGNAL(triggered()), this, SLOT(slotEditIpCamera()));

    connect(ui->viewServerPageServer, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotServerPageServerItemDoubleClicked(QModelIndex)));
    connect(m_selectionModelServerPageServer, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotServerPageServerItemSelectionChanged(QItemSelection,QItemSelection)));

    connect(ui->actionAddArea, SIGNAL(triggered()), this, SLOT(slotAddArea()));
    connect(ui->actionEditArea, SIGNAL(triggered()), this, SLOT(slotEditArea()));
    connect(ui->actionDeleteArea, SIGNAL(triggered()), this, SLOT(slotDeleteArea()));

    connect(ui->btnAppendCameraToArea, SIGNAL(clicked()), this, SLOT(slotAddCameraToArea()));
    connect(ui->btnRemoveCameraFromArea, SIGNAL(clicked()), this, SLOT(slotRemoveCameraFromArea()));

    connect(ui->viewAreaPageArea, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotAreaEditItemDoubleClicked(QModelIndex)));
    connect(ui->viewAreaPageServer, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotAreaServerItemDoubleClicked(QModelIndex)));
    connect(m_modelAreaPageArea, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(slotAreaEditItemDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(m_modelAreaPageServer, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(slotAreaServerItemDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(m_selectionModelAreaPageArea, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotAreaPageAreaItemSelectionChaned(QItemSelection, QItemSelection)));

    connect(m_selectionModelBlackPageArea, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotEditBlackAreaSelectionChanged(QItemSelection,QItemSelection)));

    connect(ui->actionAddBlackFromFile, SIGNAL(triggered()), this, SLOT(slotAddBlackFromSingleFile()));
    connect(ui->actionAddBlackFromBatch, SIGNAL(triggered()), this, SLOT(slotAddBlackFromBatchFile()));
    connect(ui->actionEditBlack, SIGNAL(triggered()), this, SLOT(slotEditBlackPerson()));
    connect(ui->actionDeleteBlack, SIGNAL(triggered()), this, SLOT(slotDeleteBlackPerson()));

    connect(ui->viewEditBlackList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotBlackInfoItemDoubleClicked(QModelIndex)));
    connect(m_selectionModelBlackPageBlack, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotBlackInfoItemSelectionChanged(QItemSelection, QItemSelection)));

    connect(ui->actionSearchPageLog, SIGNAL(triggered()), this, SLOT(slotViewSearchLog()));
    connect(ui->actionSearchPageBlack, SIGNAL(triggered()), this, SLOT(slotViewSearchBlack()));
    connect(ui->btnSearchLog, SIGNAL(clicked()), this, SLOT(slotSearchLog()));
    connect(ui->btnSetFace, SIGNAL(clicked()), this, SLOT(slotSetFace()));
    connect(ui->sldThreshold, SIGNAL(valueChanged(int)), this, SLOT(slotThresoldChanged(int)));
    connect(ui->btnSearchBlack, SIGNAL(clicked()), this, SLOT(slotSearchBlack()));

    connect(m_modelSearchPageLogArea, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(slotSearchPageLogAreaItemDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(m_modelSearchPageBlackArea, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(slotSearchPageBlackAreaItemDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(m_selectionModelSearchPageBlackList, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSearchBlackResultSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->viewSearchBlackList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotSearchBlackResultDoubleClicked(QModelIndex)));

    connect(ui->btnLogFirst, SIGNAL(clicked()), this, SLOT(slotFirstLog()));
    connect(ui->btnLogBackward, SIGNAL(clicked()), this, SLOT(slotBackwardLog()));
    connect(ui->btnLogPrev, SIGNAL(clicked()), this, SLOT(slotPrevLog()));
    connect(ui->btnLogNext, SIGNAL(clicked()), this, SLOT(slotNextLog()));
    connect(ui->btnLogForward, SIGNAL(clicked()), this, SLOT(slotForwardLog()));
    connect(ui->btnLogLast, SIGNAL(clicked()), this, SLOT(slotLastLog()));

    connect(ui->btnBlackFirst, SIGNAL(clicked()), this, SLOT(slotFirstBlack()));
    connect(ui->btnBlackBackward, SIGNAL(clicked()), this, SLOT(slotBackwardBlack()));
    connect(ui->btnBlackPrev, SIGNAL(clicked()), this, SLOT(slotPrevBlack()));
    connect(ui->btnBlackNext, SIGNAL(clicked()), this, SLOT(slotNextBlack()));
    connect(ui->btnBlackForward, SIGNAL(clicked()), this, SLOT(slotForwardBlack()));
    connect(ui->btnBlackLast, SIGNAL(clicked()), this, SLOT(slotLastBlack()));

    connect(ui->actionExportBlack, SIGNAL(triggered()), this, SLOT(slotExportBlack()));
    connect(ui->actionImportBlack, SIGNAL(triggered()), this, SLOT(slotImportBlack()));
    connect(ui->actionSendToBlack, SIGNAL(triggered()), this, SLOT(slotSendToBlack()));

    connect(btnPassword, SIGNAL(clicked()), this, SLOT(slotNewPassword()));
    connect(ui->actionNewPassword, SIGNAL(triggered()), this, SLOT(slotNewPassword()));
    connect(ui->actionChangePassword, SIGNAL(triggered()), this, SLOT(slotChangePassword()));
    connect(ui->actionRemovePassword, SIGNAL(triggered()), this, SLOT(slotRemovePassword()));
    connect(m_resultAlarmDlg, SIGNAL(closed()), this, SLOT(showResultAlarmDlg()));
}

void MainWindow::setupSockets()
{
    WSADATA w;
    int error = WSAStartup (0x0202, &w);   // Fill in WSA info
    if (error)
        return;

    if (w.wVersion != 0x0202)
        WSACleanup (); // unload ws2_32.dll
}

void MainWindow::refreshActions()
{
    ui->actionMonitoringPage->setChecked(false);
    ui->actionServer->setChecked(false);
    ui->actionMonitoringArea->setChecked(false);
    ui->actionBlackList->setChecked(false);
    ui->actionSearchPage->setChecked(false);
    if(ui->uiManager->currentIndex() == 0)
        ui->actionMonitoringPage->setChecked(true);
    else if(ui->uiManager->currentIndex() == 1)
        ui->actionServer->setChecked(true);
    else if(ui->uiManager->currentIndex() == 2)
        ui->actionMonitoringArea->setChecked(true);
    else if(ui->uiManager->currentIndex() == 3)
        ui->actionBlackList->setChecked(true);
    else if(ui->uiManager->currentIndex() == 4)
        ui->actionSearchPage->setChecked(true);

    if(m_selectionModelServerPageServer->hasSelection())
    {
        ui->actionEditServer->setEnabled(true);
        ui->actionDeleteServer->setEnabled(true);
        ui->actionEditIPCameras->setEnabled(false);

        QModelIndex index = m_selectionModelServerPageServer->selection().indexes().at(0);
        int serverIndex = m_modelServerPageServer->data(index, SERVER_INDEX_ROLE).toInt();
        if(serverIndex >= 0 && serverIndex < m_serverInfoSockets.size() && m_serverInfoSockets[serverIndex]->status() != SERVER_STOP)
            ui->actionEditIPCameras->setEnabled(true);
    }
    else
    {
        ui->actionEditServer->setEnabled(false);
        ui->actionDeleteServer->setEnabled(false);
        ui->actionEditIPCameras->setEnabled(false);
    }

    int selectedArea = selectedEditAreaIndex();
    if(selectedArea >= 0)
    {
        ui->actionEditArea->setEnabled(true);
        ui->actionDeleteArea->setEnabled(true);
    }
    else
    {
        ui->actionEditArea->setEnabled(false);
        ui->actionDeleteArea->setEnabled(false);
    }

    int selectedChanel = -1;
    if(m_selectionModelBlackPageArea->hasSelection())
    {
        QModelIndex index = m_selectionModelBlackPageArea->currentIndex();
        selectedChanel = m_modelBlackPageArea->data(index, CHANEL_INDEX_ROLE).toInt() - 1;
    }
    if(selectedChanel >= 0)
    {
        ui->actionAddBlackFromFile->setEnabled(true);
        ui->actionAddBlackFromBatch->setEnabled(true);
        ui->actionImportBlack->setEnabled(true);

        if(m_selectedBlackMetaInfo.size())
        {
            ui->actionExportBlack->setEnabled(true);
            ui->actionSendToBlack->setEnabled(true);
        }
        else
        {
            ui->actionExportBlack->setEnabled(false);
            ui->actionSendToBlack->setEnabled(false);
        }
    }
    else
    {
        ui->actionAddBlackFromFile->setEnabled(false);
        ui->actionAddBlackFromBatch->setEnabled(false);
        ui->actionExportBlack->setEnabled(false);
        ui->actionImportBlack->setEnabled(false);
        ui->actionSendToBlack->setEnabled(false);
    }

    if(m_selectionModelBlackPageBlack->hasSelection())
    {
        ui->actionEditBlack->setEnabled(true);
        ui->actionDeleteBlack->setEnabled(true);
    }
    else
    {
        ui->actionEditBlack->setEnabled(false);
        ui->actionDeleteBlack->setEnabled(false);
    }

    ui->actionNewPassword->setEnabled(false);
    ui->actionChangePassword->setEnabled(false);
    ui->actionRemovePassword->setEnabled(false);

    QString passStr = readPass();
    if(passStr.isEmpty())
        ui->actionNewPassword->setEnabled(true);
    else
    {
        ui->actionChangePassword->setEnabled(true);
        ui->actionRemovePassword->setEnabled(true);
    }
}

void MainWindow::setupCameraViews()
{
    QVBoxLayout* vLayout = new QVBoxLayout;
    for(int i = 0; i < CAMERA_VIEW_MAX_ROW; i ++)
    {
        QHBoxLayout* hLayout = new QHBoxLayout;
        for(int j = 0; j < CAMERA_VIEW_MAX_COL; j ++)
        {
            CameraSurfaceView* surfaceView = new CameraSurfaceView();
            hLayout->addWidget(surfaceView);
            hLayout->setMargin(0);
            hLayout->setSpacing(0);

            connect(surfaceView, SIGNAL(maximumChanged()), this, SLOT(slotMediaMaximumChanged()));
            connect(surfaceView, SIGNAL(selected(CameraSurfaceView*)), this, SLOT(slotMediaSelectionChanged(CameraSurfaceView*)));
            m_cameraViewItems.append(surfaceView);
        }

        vLayout->setMargin(0);
        vLayout->setSpacing(0);
        vLayout->addLayout(hLayout);
    }

    ui->viewMonitoringPageCamera->setLayout(vLayout);

    initCameraViewItem();
}

void MainWindow::initCameraViewItem()
{
    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        m_cameraViewItems[i]->stopMonitoring();

        m_cameraViewItems[i]->setMaximum(0);
        m_cameraViewItems[i]->setSelected(i == 0);
    }
}

void MainWindow::refreshCameraBar()
{
    ui->actionCamera1_1->setChecked(false);
    ui->actionCamera2_2->setChecked(false);
    ui->actionCamera3_3->setChecked(false);
    ui->actionCamera4_4->setChecked(false);
    if(m_cameraViewType == CAMERA_VIEW_1_1)
        ui->actionCamera1_1->setChecked(true);
    else if(m_cameraViewType == CAMERA_VIEW_2_2)
        ui->actionCamera2_2->setChecked(true);
    else if(m_cameraViewType == CAMERA_VIEW_3_3)
        ui->actionCamera3_3->setChecked(true);
    else if(m_cameraViewType == CAMERA_VIEW_4_4)
        ui->actionCamera4_4->setChecked(true);
}

void MainWindow::setupLogViewItems()
{
    m_sceneMonitoringPageLog->clear();
    m_logViewItem.clear();

    for(int i = 0; i < ui->spinMonitoringPageLogCount->value(); i ++)
    {
        LogViewItem* logViewItem = new LogViewItem;
        m_sceneMonitoringPageLog->addItem(logViewItem);
        m_logViewItem.append(logViewItem);

        connect(logViewItem, SIGNAL(logItemDoubleClicked(LOG_RESULT)), this, SLOT(slotLogItemDoubleClicked(LOG_RESULT)));
    }

    relocateLogViewItems();
}

void MainWindow::relocateLogViewItems()
{
    if(m_faceImageInfos.size() > ui->spinMonitoringPageLogCount->value())
        m_faceImageInfos.resize(ui->spinMonitoringPageLogCount->value());

    int rowHeight = LOG_ITEM_HEIGHT + 10;
    int minVal = qMin(m_faceImageInfos.size(), m_logViewItem.size());
    QRect sceneRect(0, 0, LOG_ITEM_WIDTH, rowHeight * minVal);
    m_sceneMonitoringPageLog->setSceneRect(sceneRect);

    for(int i = 0; i < m_logViewItem.size(); i ++)
    {
        if(i < m_faceImageInfos.size())
        {
            m_logViewItem[i]->setInfo(m_faceImageInfos[i]);
            m_logViewItem[i]->setPos(5, 5 + i * rowHeight);
        }
        else
        {
            m_logViewItem[i]->setPos(-1 * LOG_ITEM_WIDTH, -1 * rowHeight);
        }
    }

    m_sceneMonitoringPageLog->update();

    ui->lblMonitoringPageCurLogCount->setText(QString::number(m_faceImageInfos.size()) + " /   ");
}

void MainWindow::relocateCameraViewItems()
{
    int hasMaximum = -1;

    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        if(m_cameraViewItems[i]->isMaximum())
            hasMaximum = i;
    }

    if(hasMaximum >= 0)
    {
        for(int i = 0; i < m_cameraViewItems.size(); i ++)
            m_cameraViewItems[i]->setVisible(hasMaximum == i);
    }
    else
    {
        for(int i = 0; i < CAMERA_VIEW_MAX_ROW; i ++)
        {
            for(int j = 0; j < CAMERA_VIEW_MAX_COL; j ++)
            {
                m_cameraViewItems[i * CAMERA_VIEW_MAX_COL + j]->setVisible(i < m_cameraViewType + 1 && j < m_cameraViewType + 1);
            }
        }
    }

}


void MainWindow::refreshMonitoringPageArea()
{
    m_monitoringPageAreaModel->removeRows(0, m_monitoringPageAreaModel->rowCount());

    for(int i = 0; i < m_areaInfos.size(); i ++)
    {
        MonitoringAreaInfo monitoringAreaInfo = m_areaInfos[i];
        QStandardItem* monitoringAreaInfoItem = new QStandardItem;
        monitoringAreaInfoItem->setText(monitoringAreaInfo.areaName);
        monitoringAreaInfoItem->setIcon(QIcon(QPixmap(":/images/monitoring_area.png")));

        monitoringAreaInfoItem->setData(i, AREA_INDEX_ROLE);

        for(int j = 0; j < monitoringAreaInfo.serverIndexs.size(); j ++)
        {
            int selectedServerIndex = monitoringAreaInfo.serverIndexs[j];
            int selectedChanelIndex = monitoringAreaInfo.chanelIndexs[j];

            ServerInfo serverInfo = m_serverInfoSockets[selectedServerIndex]->serverInfo();
            int chanelStatus = m_serverInfoSockets[selectedServerIndex]->getChanelStatus(selectedChanelIndex);
            int serverStatus = m_serverInfoSockets[selectedServerIndex]->status();

            QString serverInfoName = serverInfo.serverName;
            QString cameraItemText = serverInfoName + " (" + StringTable::Str_Chanel + QString::number(selectedChanelIndex + 1) + ")";
            QStandardItem* cameraItem = new QStandardItem;
            cameraItem->setText(cameraItemText);

            if(chanelStatus == CHANEL_STATUS_STOP)
            {
                if(serverStatus == SERVER_STOP)
                    cameraItem->setEnabled(false);
                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
            }
            else
                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));


            cameraItem->setData(i, AREA_INDEX_ROLE);
            cameraItem->setData(selectedServerIndex, SERVER_INDEX_ROLE);
            cameraItem->setData(selectedChanelIndex, CHANEL_INDEX_ROLE);

            monitoringAreaInfoItem->appendRow(cameraItem);
        }

        m_monitoringPageAreaModel->appendRow(monitoringAreaInfoItem);
        ui->viewMonitoringPageArea->setExpanded(m_monitoringPageAreaModel->index(i, 0), true);
    }
}

void MainWindow::refreshMonitoringPageBlackResult()
{
    m_monitoringPageBlackResultModel->removeRows(0, m_monitoringPageBlackResultModel->rowCount());

    for(int i = 0; i < m_blackRecogResults.size(); i ++)
    {
        QString addNameStr = m_blackRecogResults[i][0].name;
        QDateTime addDateTime = m_blackRecogResults[i][0].dateTime;

        QStandardItem* existItem = NULL;
        int selectedRow = -1;
        for(int j = 0; j < m_monitoringPageBlackResultModel->rowCount(); j ++)
        {
            QStandardItem* firstItem = m_monitoringPageBlackResultModel->item(j);
            int diffSec = firstItem->data(GROUP_DATETIME_ROLE).toDateTime().secsTo(addDateTime);
            if(firstItem->data(GROUP_NAME_ROLE).toString() == addNameStr &&
                     diffSec < LAST_LOG_DIFF && diffSec >= 0)
            {
                existItem = firstItem;
                selectedRow = j;
                break;
            }
        }

        if(existItem)
        {
            float oldSimilarity = m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, 4)).toFloat();
            if(oldSimilarity < m_blackRecogResults[i][0].similiarity)
            {
                QList<QStandardItem*> rowItems;
                for(int j = 0; j < 9; j ++)
                {
                    QStandardItem* item = new QStandardItem;
                    if(j != 0)
                        item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j)), Qt::DisplayRole);
                    else
                        item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j), GROUP_DATETIME_ROLE), GROUP_DATETIME_ROLE);
                    item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j), LOG_UID_ROLE), LOG_UID_ROLE);

                    rowItems.append(item);
                }

                existItem->insertRow(0, rowItems);

                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), m_blackRecogResults[i][0].dateTime, GROUP_DATETIME_ROLE);

                QString personTypeStr;
                if(m_blackRecogResults[i][0].personType == 0)
                {
                    personTypeStr = StringTable::Str_White;
                    m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::green), Qt::BackgroundColorRole);
                }
                else if(m_blackRecogResults[i][0].personType == 1)
                {
                    personTypeStr = StringTable::Str_Black;
                    m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::red), Qt::BackgroundColorRole);
                }
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 1), personTypeStr);
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 2), m_blackRecogResults[i][0].areaName);
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 3), m_blackRecogResults[i][0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 4), m_blackRecogResults[i][0].similiarity);

                QString genderStr;
                if(m_blackRecogResults[i][0].gender == 0)
                    genderStr = StringTable::Str_Male;
                else
                    genderStr = StringTable::Str_Female;

                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 5), genderStr);
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 6), m_blackRecogResults[i][0].birthday);
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 7), m_blackRecogResults[i][0].address);
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 8), m_blackRecogResults[i][0].description);

                for(int j = 0; j < 9; j ++)
                    m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, j), m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
            }
            else
            {
                QList<QStandardItem*> rowItems;
                QStandardItem* item = new QStandardItem;
                item->setData(m_blackRecogResults[i][0].dateTime, GROUP_DATETIME_ROLE);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                QString personTypeStr;
                if(m_blackRecogResults[i][0].personType == 0)
                {
                    personTypeStr = StringTable::Str_White;
                    m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::green), Qt::BackgroundColorRole);
                }
                else if(m_blackRecogResults[i][0].personType == 1)
                {
                    personTypeStr = StringTable::Str_Black;
                    m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::red), Qt::BackgroundColorRole);
                }

                item = new QStandardItem;
                item->setData(personTypeStr, Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                item = new QStandardItem;
                item->setData(m_blackRecogResults[i][0].areaName, Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                item = new QStandardItem;
                item->setData(m_blackRecogResults[i][0].dateTime.toString("yyyy-MM-dd hh:mm:ss"), Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                item = new QStandardItem;
                item->setData(m_blackRecogResults[i][0].similiarity, Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                QString genderStr;
                if(m_blackRecogResults[i][0].gender == 0)
                    genderStr = StringTable::Str_Male;
                else
                    genderStr = StringTable::Str_Female;

                item = new QStandardItem;
                item->setData(genderStr, Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                item = new QStandardItem;
                item->setData(m_blackRecogResults[i][0].birthday, Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                item = new QStandardItem;
                item->setData(m_blackRecogResults[i][0].address, Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                item = new QStandardItem;
                item->setData(m_blackRecogResults[i][0].description, Qt::DisplayRole);
                item->setData(m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
                rowItems.append(item);

                existItem->insertRow(0, rowItems);

                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), m_blackRecogResults[i][0].dateTime, GROUP_DATETIME_ROLE);
            }
        }
        else
        {
            int rowIndex = 0;
            m_monitoringPageBlackResultModel->insertRow(rowIndex);

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), m_blackRecogResults[i][0].name);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), m_blackRecogResults[i][0].name, GROUP_NAME_ROLE);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), m_blackRecogResults[i][0].dateTime, GROUP_DATETIME_ROLE);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), m_blackAlarmId, ALRM_ID_ROLE);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), 0, ALRM_FLAG_ROLE);
            m_blackAlarmId ++;

            QString personTypeStr;
            if(m_blackRecogResults[i][0].personType == 0)
            {
                personTypeStr = StringTable::Str_White;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), QColor(Qt::green), Qt::BackgroundColorRole);
            }
            else if(m_blackRecogResults[i][0].personType == 1)
            {
                personTypeStr = StringTable::Str_Black;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), QColor(Qt::red), Qt::BackgroundColorRole);
            }
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 1), personTypeStr);

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 2), m_blackRecogResults[i][0].areaName);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 3), m_blackRecogResults[i][0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 4), m_blackRecogResults[i][0].similiarity);

            QString genderStr;
            if(m_blackRecogResults[i][0].gender == 0)
                genderStr = StringTable::Str_Male;
            else
                genderStr = StringTable::Str_Female;

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 5), genderStr);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 6), m_blackRecogResults[i][0].birthday);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 7), m_blackRecogResults[i][0].address);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 8), m_blackRecogResults[i][0].description);

            for(int j = 0; j < 9; j ++)
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, j), m_blackRecogResults[i][0].logUID, LOG_UID_ROLE);
        }
    }

    if(m_monitoringPageBlackResultModel->rowCount() > N_MAX_BLACK_RECOG_RESULT)
    {
        for(int k = N_MAX_BLACK_RECOG_RESULT; k < m_monitoringPageBlackResultModel->rowCount(); k ++)
        {
            QStandardItem* lastItem = m_monitoringPageBlackResultModel->item(k);
            for(int i = 0; i < lastItem->rowCount(); i ++)
            {
                QStandardItem* childItem = lastItem->child(i);
                QString logUID = childItem->data(LOG_UID_ROLE).toString();

                for(int j = 0; j < m_blackRecogResults.size(); j ++)
                {
                    if(m_blackRecogResults[j][0].logUID == logUID)
                    {
                        m_blackRecogResults.remove(j);
                        break;
                    }
                }
            }

            QString logUID = lastItem->data(LOG_UID_ROLE).toString();
            for(int j = 0; j < m_blackRecogResults.size(); j ++)
            {
                if(m_blackRecogResults[j][0].logUID == logUID)
                {
                    m_blackRecogResults.remove(j);
                    break;
                }
            }

            m_monitoringPageBlackResultModel->removeRow(k);
            k --;
        }
    }

    qDebug() << "black row count" << m_monitoringPageBlackResultModel->rowCount() << m_blackRecogResults.size();
    if(m_monitoringPageBlackResultModel->rowCount())
    {
    ui->viewMonitoringPageBlackResult->clearSelection();
    ui->viewMonitoringPageBlackResult->setCurrentIndex(m_monitoringPageBlackResultModel->index(0, 0));
}
}

void MainWindow::refreshServers()
{
    m_modelServerPageServer->removeRows(0, m_modelServerPageServer->rowCount());

    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        ServerInfo serverInfo = m_serverInfoSockets[i]->serverInfo();
        int serverStatus = m_serverInfoSockets[i]->status();

        m_modelServerPageServer->insertRow(m_modelServerPageServer->rowCount());
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 0), serverInfo.serverName);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 0), Qt::AlignCenter, Qt::TextAlignmentRole);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 0), i, SERVER_INDEX_ROLE);
        if(serverStatus == SERVER_STOP)
            m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 0), QIcon(QPixmap(":/images/machine_disable1.png")), Qt::DecorationRole);
        else
            m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 0), QIcon(QPixmap(":/images/machine.png")), Qt::DecorationRole);

        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 1), serverInfo.ipAddress);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 1), Qt::AlignCenter, Qt::TextAlignmentRole);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 1), i, SERVER_INDEX_ROLE);

        if(serverInfo.ipCameraInfos.size())
            m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 2), serverInfo.ipCameraInfos.size());
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 2), Qt::AlignCenter, Qt::TextAlignmentRole);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 2), i, SERVER_INDEX_ROLE);

        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 3), serverInfo.serverTypeStr);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 3), Qt::AlignCenter, Qt::TextAlignmentRole);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 3), i, SERVER_INDEX_ROLE);

        if(serverStatus != SERVER_STOP)
            m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 4), StringTable::Str_Connected);
        else
            m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 4), StringTable::Str_Disconnected);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 4), Qt::AlignCenter, Qt::TextAlignmentRole);
        m_modelServerPageServer->setData(m_modelServerPageServer->index(m_modelServerPageServer->rowCount() - 1, 4), i, SERVER_INDEX_ROLE);
    }

    m_modelAreaPageServer->removeRows(0, m_modelAreaPageServer->rowCount());

    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        ServerInfo serverInfo = m_serverInfoSockets[i]->serverInfo();
        QStandardItem* serverItem = new QStandardItem;
        serverItem->setText(serverInfo.serverName);

        int serverStatus = m_serverInfoSockets[i]->status();
        if(serverStatus == SERVER_STOP)
            serverItem->setIcon(QIcon(QPixmap(":/images/machine_disable1.png")));
        else
            serverItem->setIcon(QIcon(QPixmap(":/images/machine.png")));

        for(int j = 0; j < serverInfo.ipCameraInfos.size(); j ++)
        {
            QString chanelIndexStr;
            chanelIndexStr.sprintf(" (%d)", j + 1);

            int chanelStatus = m_serverInfoSockets[i]->getChanelStatus(j);

            QStandardItem* chanelItem = new QStandardItem;
            chanelItem->setText(StringTable::Str_Chanel + chanelIndexStr);
            if(chanelStatus == CHANEL_STATUS_STOP)
                chanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
            else
                chanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

            chanelItem->setCheckable(true);

            serverItem->appendRow(chanelItem);

            serverItem->setCheckable(true);
        }

        m_modelAreaPageServer->appendRow(serverItem);
        ui->viewAreaPageServer->setExpanded(m_modelAreaPageServer->index(i, 0), true);
    }
}

void MainWindow::addServerSocket(ServerInfo serverInfo)
{
    ServerInfoSocket* serverInfoSocket = new ServerInfoSocket;
    m_serverInfoSockets.append(serverInfoSocket);

    connect(serverInfoSocket, SIGNAL(statusChanged(int, int)), this, SLOT(slotServerStatusChanged(int, int)));
    connect(serverInfoSocket, SIGNAL(chanelStatusChanged(int, int, int)), this, SLOT(slotServerChanelStatusChanged(int, int, int)));
    connect(serverInfoSocket, SIGNAL(blackInfoChanged(int,int)), this, SLOT(slotBlackInfoChanged(int, int)));

    serverInfoSocket->startSocket(serverInfo, m_serverInfoSockets.size() - 1);
}

void MainWindow::addLogFaceReceiveSocket(int serverIndex, int chanelIndex)
{
    if(serverIndex < 0 || serverIndex >= m_serverInfoSockets.size())
        return;

    LogResultReceiveSocket* faceImageReceiveSocket = new LogResultReceiveSocket;
    faceImageReceiveSocket->setInfo(m_serverInfoSockets[serverIndex], chanelIndex);
    m_logImageReceiveSockets.append(faceImageReceiveSocket);

    connect(faceImageReceiveSocket, SIGNAL(receivedLogResult(LOG_RESULT)), this, SLOT(slotReceiveLogResult(LOG_RESULT)));

    faceImageReceiveSocket->start();
}

void MainWindow::addBlackRecogResultReceiveSocket(int serverIndex, int chanelIndex)
{
    if(serverIndex < 0 || serverIndex >= m_serverInfoSockets.size())
        return;

    BlackRecogResultReceiveSocket* blackRecogResultReceiveSocket = new BlackRecogResultReceiveSocket;
    blackRecogResultReceiveSocket->setInfo(m_serverInfoSockets[serverIndex], chanelIndex);
    m_blackRecogResultReceiveSockets.append(blackRecogResultReceiveSocket);

    connect(blackRecogResultReceiveSocket, SIGNAL(receivedBlackRecogResult(QVector<BLACK_RECOG_RESULT>)), this, SLOT(slotReceiveBlackRecogResults(QVector<BLACK_RECOG_RESULT>)));

    blackRecogResultReceiveSocket->start();
}

void MainWindow::refreshAreaPageArea()
{
    m_modelAreaPageArea->removeRows(0, m_modelAreaPageArea->rowCount());

    for(int i = 0; i < m_areaInfos.size(); i ++)
    {
        MonitoringAreaInfo monitoringAreaInfo = m_areaInfos[i];
        QStandardItem* monitoringAreaInfoItem = new QStandardItem;
        monitoringAreaInfoItem->setText(monitoringAreaInfo.areaName);
        monitoringAreaInfoItem->setIcon(QIcon(QPixmap(":/images/monitoring_area.png")));
        monitoringAreaInfoItem->setData(i + 1, AREA_INDEX_ROLE);

        for(int j = 0; j < monitoringAreaInfo.serverIndexs.size(); j ++)
        {
            int serverIndex = monitoringAreaInfo.serverIndexs[j];
            int chanelIndex = monitoringAreaInfo.chanelIndexs[j];

            ServerInfo serverInfo = m_serverInfoSockets[serverIndex]->serverInfo();
            int chanelStatus = m_serverInfoSockets[serverIndex]->getChanelStatus(chanelIndex);

            QString serverInfoName = serverInfo.serverName;
            QString cameraItemText = serverInfoName + " (" + StringTable::Str_Chanel + QString::number(monitoringAreaInfo.chanelIndexs[j] + 1) + ")";
            QStandardItem* cameraItem = new QStandardItem;
            cameraItem->setText(cameraItemText);

            if(chanelStatus == CHANEL_STATUS_STOP)
                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
            else
                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

            cameraItem->setCheckable(true);

            monitoringAreaInfoItem->appendRow(cameraItem);
            monitoringAreaInfoItem->setCheckable(true);
        }

        m_modelAreaPageArea->appendRow(monitoringAreaInfoItem);
        ui->viewAreaPageArea->setExpanded(m_modelAreaPageArea->index(i, 0), true);
    }
}

int MainWindow::selectedEditAreaIndex()
{
    if(!m_selectionModelAreaPageArea->hasSelection())
        return -1;

    QModelIndex index = m_selectionModelAreaPageArea->currentIndex();
    int areaIndex = m_modelAreaPageArea->data(index, AREA_INDEX_ROLE).toInt() - 1;
    return areaIndex;
}

void MainWindow::editSelectedIpCamera(int serverIndex, int chanelIndex)
{
    return;
    if(serverIndex < 0 || serverIndex >= m_serverInfoSockets.size())
        return;

    ServerInfo serverInfo = m_serverInfoSockets[serverIndex]->serverInfo();
    if(chanelIndex < 0 || chanelIndex >= serverInfo.ipCameraInfos.size())
        return;

    IpCameraOneSettingDlg dlg;
    dlg.setIpCameraInfo(serverInfo.ipCameraInfos[chanelIndex]);
    if(dlg.exec() == 0)
        return;

    IpCameraInfo cameraInfo = dlg.ipCameraInfo();
    serverInfo.ipCameraInfos[chanelIndex] = cameraInfo;
    m_serverInfoSockets[serverIndex]->setIpCameraInfos(serverInfo.ipCameraInfos);
}

void MainWindow::refreshBlackPageArea()
{
    m_modelBlackPageArea->removeRows(0, m_modelBlackPageArea->rowCount());

    for(int i = 0; i < m_areaInfos.size(); i ++)
    {
        MonitoringAreaInfo monitoringAreaInfo = m_areaInfos[i];
        QStandardItem* monitoringAreaInfoItem = new QStandardItem;
        monitoringAreaInfoItem->setText(monitoringAreaInfo.areaName);
        monitoringAreaInfoItem->setIcon(QIcon(QPixmap(":/images/monitoring_area.png")));
        monitoringAreaInfoItem->setData(i + 1, AREA_INDEX_ROLE);

        for(int j = 0; j < monitoringAreaInfo.serverIndexs.size(); j ++)
        {
            int selectedServerIndex = monitoringAreaInfo.serverIndexs[j];
            int selectedChanelIndex = monitoringAreaInfo.chanelIndexs[j];

            ServerInfo serverInfo = m_serverInfoSockets[selectedServerIndex]->serverInfo();
            int serverStatus = m_serverInfoSockets[selectedServerIndex]->status();
            int chanelStatus = m_serverInfoSockets[selectedServerIndex]->getChanelStatus(selectedChanelIndex);

            QString serverInfoName = serverInfo.serverName;
            QString cameraItemText = serverInfoName + " (" + StringTable::Str_Chanel + QString::number(selectedChanelIndex + 1) + ")";
            QStandardItem* cameraItem = new QStandardItem;
            if(chanelStatus == CHANEL_STATUS_STOP)
            {
                if(serverStatus == SERVER_STOP)
                    cameraItem->setEnabled(false);

                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
            }
            else
                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

            if(serverStatus != SERVER_STOP)
                cameraItemText += " - " + QString::number(serverInfo.ipCameraInfos[selectedChanelIndex].blackInfoSize);

            cameraItem->setText(cameraItemText);
            cameraItem->setData(selectedChanelIndex + 1, CHANEL_INDEX_ROLE);
            cameraItem->setData(selectedServerIndex + 1, SERVER_INDEX_ROLE);


            monitoringAreaInfoItem->appendRow(cameraItem);
        }

        m_modelBlackPageArea->appendRow(monitoringAreaInfoItem);
        ui->viewEditBlackArea->setExpanded(m_modelBlackPageArea->index(i, 0), true);
    }
}

void MainWindow::refreshSelectedBlackMetaInfos()
{
    m_modelBlackPageBlack->removeRows(0, m_modelBlackPageBlack->rowCount());

    for(int i = 0; i < m_selectedBlackMetaInfo.size(); i ++)
    {
        BLACK_PERSON blackPerson = m_selectedBlackMetaInfo[i];

        m_modelBlackPageBlack->insertRow(m_modelBlackPageBlack->rowCount());
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 0), blackPerson.name);
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 0), Qt::AlignCenter, Qt::TextAlignmentRole);

        QString personTypeStr;
        if(blackPerson.personType == 0)
        {
            personTypeStr = StringTable::Str_White;
            m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 0), QColor(Qt::green), Qt::BackgroundColorRole);
        }
        else if(blackPerson.personType == 1)
        {
            personTypeStr = StringTable::Str_Black;
            m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 0), QColor(Qt::red), Qt::BackgroundColorRole);
        }

        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 1), personTypeStr);
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 1), Qt::AlignCenter, Qt::TextAlignmentRole);


        QString genderStr;
        if(blackPerson.gender == 0)
            genderStr = StringTable::Str_Male;
        else if(blackPerson.gender == 1)
            genderStr = StringTable::Str_Female;
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 2), genderStr);
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 2), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 3), blackPerson.birthDay);
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 3), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 4), blackPerson.address);
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 4), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 5), blackPerson.description);
        m_modelBlackPageBlack->setData(m_modelBlackPageBlack->index(m_modelBlackPageBlack->rowCount() - 1, 5), Qt::AlignCenter, Qt::TextAlignmentRole);
    }
}

void MainWindow::refreshSelectedBlackInfoByArea()
{
    stopBlackInfoReceive();

    refreshActions();
    if(!m_selectionModelBlackPageArea->hasSelection())
        return;

    QModelIndex currentIndex = m_selectionModelBlackPageArea->currentIndex();

    int selectedServerIndex = currentIndex.data(SERVER_INDEX_ROLE).toInt() - 1;
    int selectedChanelIndex = currentIndex.data(CHANEL_INDEX_ROLE).toInt() - 1;

    if(selectedServerIndex < 0 || selectedServerIndex >= m_serverInfoSockets.size() || selectedChanelIndex < 0)
        return;

    m_selectedServerIndexOfEditBlack = selectedServerIndex;
    m_selectedChanelIndexOfEditBlack = selectedChanelIndex;

    m_blackInfoReceiveSocket = new BlackInfoReceiveSocket;
    m_blackInfoReceiveSocket->startSocket(m_serverInfoSockets[selectedServerIndex]->serverInfo(), selectedChanelIndex);
    connect(m_blackInfoReceiveSocket, SIGNAL(receivedBlackInfos(QVector<BLACK_PERSON>)), this, SLOT(slotReceivedSelectedBlackInfos(QVector<BLACK_PERSON>)));
    ui->progressBarBlack->show();
//    QApplication::setOverrideCursor(Qt::WaitCursor);
    refreshActions();
}

void MainWindow::stopBlackInfoReceive()
{
    if(m_blackInfoReceiveSocket)
    {
        m_blackInfoReceiveSocket->stopSocket();
        delete m_blackInfoReceiveSocket;
        m_blackInfoReceiveSocket = NULL;
    }

    m_selectedBlackMetaInfo.clear();
    refreshSelectedBlackMetaInfos();
    m_selectedServerIndexOfEditBlack = -1;
    m_selectedChanelIndexOfEditBlack = -1;

    ui->progressBarBlack->hide();
//    QApplication::restoreOverrideCursor();
}

void MainWindow::stopBlackInfoDelete()
{
    if(m_deletBlackListSocket)
    {
        m_deletBlackListSocket->stopDelete();
        delete m_deletBlackListSocket;
        m_deletBlackListSocket = NULL;
    }
}

void MainWindow::stopAddBlackInfoSocket()
{
    if(m_addBlackInfoSocket)
    {
        m_addBlackInfoSocket->stopSocket();
        delete m_addBlackInfoSocket;
        m_addBlackInfoSocket = NULL;
    }
}

void MainWindow::stopEditBlackInfoSocket()
{
    if(m_editBlackInfoSocket)
    {
        m_editBlackInfoSocket->stopSocket();
        delete m_editBlackInfoSocket;
        m_editBlackInfoSocket = NULL;
    }
}

void MainWindow::slotViewMonitoringPage()
{
    ui->uiManager->setCurrentIndex(0);
    ui->dateTimeSearchLogStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchLogEnd->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeSearchBlackStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchBlackEnd->setDateTime(QDateTime::currentDateTime());

    refreshActions();
}

void MainWindow::slotViewServerPage()
{
    ui->uiManager->setCurrentIndex(1);
    refreshActions();
}

void MainWindow::slotViewAreaPage()
{
    ui->uiManager->setCurrentIndex(2);
    refreshActions();
}

void MainWindow::slotViewBlackListPage()
{
    ui->uiManager->setCurrentIndex(3);
    refreshActions();
}

void MainWindow::slotViewSearchPage()
{
    ui->uiManager->setCurrentIndex(4);
    refreshActions();
}

void MainWindow::slotAbout()
{
    AboutDlg dlg;
    dlg.exec();
}

void MainWindow::slotViewCamera1_1()
{
    m_cameraViewType = CAMERA_VIEW_1_1;
    initCameraViewItem();
    refreshCameraBar();

    relocateCameraViewItems();
}

void MainWindow::slotViewCamera2_2()
{
    m_cameraViewType = CAMERA_VIEW_2_2;
    initCameraViewItem();
    refreshCameraBar();

    relocateCameraViewItems();
}

void MainWindow::slotViewCamera3_3()
{
    m_cameraViewType = CAMERA_VIEW_3_3;
    initCameraViewItem();
    refreshCameraBar();

    relocateCameraViewItems();
}

void MainWindow::slotViewCamera4_4()
{
    m_cameraViewType = CAMERA_VIEW_4_4;
    initCameraViewItem();
    refreshCameraBar();

    relocateCameraViewItems();
}

void MainWindow::slotCapture()
{
    int exist = -1;
    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        if(m_cameraViewItems[i]->isSelected())
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
        return;

    QImage capturedImage = m_cameraViewItems[exist]->currentImage();
    if(capturedImage.isNull())
        return;

    int selectedServerIndex = -1;
    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        if(m_serverInfoSockets[i]->status() != SERVER_STOP)
        {
            selectedServerIndex = i;
            break;
        }

    }

    AddBlackListFromCapturedDlg dlg;
    dlg.setInfo(m_areaInfos, m_serverInfoSockets, selectedServerIndex, 0, capturedImage, 1);
    dlg.exec();

    refreshSelectedBlackInfoByArea();
    refreshActions();

}

void MainWindow::slotMonitoringPageAreaItemDoubleClicked(QModelIndex modelIndex)
{
    CameraSurfaceView* selectedMediaItem = NULL;
    int selectedMediaIndex = -1;
    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        if(m_cameraViewItems[i]->isSelected())
        {
            selectedMediaItem = m_cameraViewItems[i];
            selectedMediaIndex = i;
            break;
        }
    }

    if(selectedMediaItem == NULL)
        return;

    QStandardItem* standardItem = m_monitoringPageAreaModel->itemFromIndex(modelIndex);
    if(standardItem->parent() == NULL/* || !standardItem->isEnabled()*/)
        return;

    int selectedServerIndex = standardItem->data(SERVER_INDEX_ROLE).toInt();
    int selectedChanelIndex = standardItem->data(CHANEL_INDEX_ROLE).toInt();
    int selectedMonitoringAreaIndex = standardItem->data(AREA_INDEX_ROLE).toInt();

    int exist = -1;
    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        if(m_cameraViewItems[i]->serverInfoSocket() == m_serverInfoSockets[selectedServerIndex] &&
                m_cameraViewItems[i]->chanelIndex() == selectedChanelIndex)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
    {
        selectedMediaItem->stopMonitoring();

        selectedMediaItem->startMonitoring(m_serverInfoSockets[selectedServerIndex], selectedChanelIndex, m_areaInfos[selectedMonitoringAreaIndex].areaName, selectedMediaIndex);

        if(selectedMediaItem->isMaximum())
        {
            selectedMediaItem->setMaximum(0);
            relocateCameraViewItems();
        }
    }
}

void MainWindow::slotMonitoringPageAreaItemContext(QPoint pos)
{
//    return;
    QModelIndex modelIndex = ui->viewMonitoringPageArea->indexAt(pos);
    QStandardItem* standardItem = m_monitoringPageAreaModel->itemFromIndex(modelIndex);
    if(!standardItem->parent() || !standardItem->isEnabled())
        return;

    m_monitoringAreaContextMenu->exec(ui->viewMonitoringPageArea->mapToGlobal(QPoint(pos.x(), pos.y() + 20)));
}

void MainWindow::slotMonitoringPageCameraSetting()
{
    if(!m_monitoringPageAreaSelectionModel->hasSelection())
        return;


    QModelIndex modelIndex = m_monitoringPageAreaSelectionModel->currentIndex();
    QStandardItem* standardItem = m_monitoringPageAreaModel->itemFromIndex(modelIndex);
    if(!standardItem->parent())
        return;

    int serverIndex = standardItem->data(SERVER_INDEX_ROLE).toInt();
    int chanelIndex = standardItem->data(CHANEL_INDEX_ROLE).toInt();
    if(chanelIndex < 0 || chanelIndex >= m_serverInfoSockets[serverIndex]->serverInfo().ipCameraInfos.size())
        return;

    MonitoringIpCameraSettngDlg dlg;
    dlg.setChanelInfo(m_serverInfoSockets[serverIndex]->serverInfo(), chanelIndex);
    int ret = dlg.exec();
    if(ret)
    {
        IpCameraInfo ipCameraInfo = dlg.ipCameraInfo();
        m_serverInfoSockets[serverIndex]->setIpCameraInfo(ipCameraInfo, chanelIndex);
    }
}

void MainWindow::slotMonitoringPageSurveillanceSetting()
{
    if(!m_monitoringPageAreaSelectionModel->hasSelection())
        return;


    QModelIndex modelIndex = m_monitoringPageAreaSelectionModel->currentIndex();
    QStandardItem* standardItem = m_monitoringPageAreaModel->itemFromIndex(modelIndex);
    if(!standardItem->parent())
        return;

    int chanelIndex = standardItem->data(CHANEL_INDEX_ROLE).toInt();
    if(chanelIndex < 0 || chanelIndex >= m_serverInfoSockets[standardItem->data(SERVER_INDEX_ROLE).toInt()]->serverInfo().ipCameraInfos.size())
        return;

    MonitoringServeillanceSettingDlg dlg;
    dlg.setInfo(m_serverInfoSockets[standardItem->data(SERVER_INDEX_ROLE).toInt()], standardItem->data(CHANEL_INDEX_ROLE).toInt());
    dlg.exec();
}

void MainWindow::slotBlackResultSelectionChanged(QItemSelection,QItemSelection)
{
    if(!m_monitoringPageBlackResultSelectionModel->hasSelection())
        return;

    QModelIndex index = m_monitoringPageBlackResultSelectionModel->currentIndex();
    QStandardItem* selectedItem = m_monitoringPageBlackResultModel->itemFromIndex(index);
    if(selectedItem == NULL)
        return;

    QString logUID = selectedItem->data(LOG_UID_ROLE).toString();
    for(int i = 0; i < m_blackRecogResults.size(); i ++)
    {
        if(m_blackRecogResults[i][0].logUID == logUID)
        {
            ui->viewMonitoringPageBlackResultImage->setBlackResult(m_blackRecogResults[i]);
            break;
        }
    }
}


void MainWindow::slotBlackResultDoubleClicked(QModelIndex)
{
    if(!m_monitoringPageBlackResultSelectionModel->hasSelection())
        return;

    QModelIndex index = m_monitoringPageBlackResultSelectionModel->currentIndex();
    QStandardItem* selectedItem = m_monitoringPageBlackResultModel->itemFromIndex(index);
    if(selectedItem == NULL)
        return;

    int selectedIndex = -1;
    QString logUID = selectedItem->data(LOG_UID_ROLE).toString();
    for(int i = 0; i < m_blackRecogResults.size(); i ++)
    {
        if(m_blackRecogResults[i][0].logUID == logUID)
        {
            selectedIndex = i;
            break;
            break;
        }
    }

    if(selectedIndex < 0)
        return;

    int exist = -1;
    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        if(m_serverInfoSockets[i]->serverInfo().serverUID == m_blackRecogResults[selectedIndex][0].serverUID)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
        return;

    LogSelectDlg dlg;
    dlg.setInfo(m_areaInfos, m_serverInfoSockets, m_blackRecogResults[selectedIndex]);
    dlg.exec();

    refreshSelectedBlackInfoByArea();
}

void MainWindow::slotMediaSelectionChanged(CameraSurfaceView* selectedView)
{
    for(int i = 0; i < m_cameraViewItems.size(); i ++)
        m_cameraViewItems[i]->setSelected(m_cameraViewItems[i] == selectedView);
}

void MainWindow::slotMediaMaximumChanged()
{
    relocateCameraViewItems();
}

void MainWindow::slotReceiveLogResult(LOG_RESULT logResult)
{
//    logResult.areaName = getAreaNameFromUID(m_areaInfos, m_serverInfoSockets, logResult.serverUID, logResult.chanelIndex);
    int serverIndex = getServerIndexFromUID(m_serverInfoSockets, logResult.serverUID);
    QString areaName = getAreaNameFromUID(m_areaInfos, m_serverInfoSockets, logResult.serverUID, logResult.chanelIndex);
    if(serverIndex < 0)
        return;

    QString serverInfoStr;
    serverInfoStr.sprintf("%s - %s (%s %d)", areaName.toUtf8().data(),
                          getServerNameFromUID(m_serverInfoSockets, logResult.serverUID).toUtf8().data(),
                          StringTable::Str_Chanel.toUtf8().data(), logResult.chanelIndex + 1);

    logResult.areaName = serverInfoStr;

    int exist = -1;
    for(int i = 0; i < m_faceImageInfos.size(); i ++)
    {
        if(m_faceImageInfos[i].logUID == logResult.logUID)
        {
            exist = i;
            break;
        }
    }

    if(exist >= 0)
    {
        m_faceImageInfos.remove(exist);
        m_faceImageInfos.insert(0, logResult);
    }
    else
        m_faceImageInfos.insert(0, logResult);

    relocateLogViewItems();
}

void MainWindow::slotReceiveBlackRecogResults(QVector<BLACK_RECOG_RESULT> blackRecogResult)
{
    if(blackRecogResult.size())
    {
        WarningPlayer::play(blackRecogResult[0].personType);
    }
    for(int i = 0; i < blackRecogResult.size(); i ++)
    {
        int serverIndex = getServerIndexFromUID(m_serverInfoSockets, blackRecogResult[0].serverUID);
        QString areaName = getAreaNameFromUID(m_areaInfos, m_serverInfoSockets, blackRecogResult[0].serverUID, blackRecogResult[0].chanelIndex);
        if(serverIndex < 0)
            continue;

        QString serverInfoStr;
        serverInfoStr.sprintf("%s - %s (%s %d)", areaName.toUtf8().data(),
                              getServerNameFromUID(m_serverInfoSockets, blackRecogResult[0].serverUID).toUtf8().data(),
                              StringTable::Str_Chanel.toUtf8().data(), blackRecogResult[0].chanelIndex + 1);

        blackRecogResult[i].areaName = serverInfoStr;
    }

    for(int i = 0; i < m_blackRecogResults.size(); i ++)
    {
        if(m_blackRecogResults[i][0].logUID == blackRecogResult[0].logUID)
            return;
    }
    m_blackRecogResults.append(blackRecogResult);

    QString addNameStr = blackRecogResult[0].name;
    QDateTime addDateTime = blackRecogResult[0].dateTime;

    QStandardItem* existItem = NULL;
    int selectedRow = -1;
    for(int j = 0; j < m_monitoringPageBlackResultModel->rowCount(); j ++)
    {
        QStandardItem* firstItem = m_monitoringPageBlackResultModel->item(j);
        int diffSec = firstItem->data(GROUP_DATETIME_ROLE).toDateTime().secsTo(addDateTime);
        if(firstItem->data(GROUP_NAME_ROLE).toString() == addNameStr &&
                 diffSec < LAST_LOG_DIFF && diffSec >= 0)
        {
            existItem = firstItem;
            selectedRow = j;
            break;
        }
    }
    if(existItem)
    {
        float oldSimilarity = m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, 4)).toFloat();
        if(oldSimilarity < blackRecogResult[0].similiarity)
        {
            QList<QStandardItem*> rowItems;
            for(int j = 0; j < 9; j ++)
            {
                QStandardItem* item = new QStandardItem;
                if(j != 0)
                    item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j)), Qt::DisplayRole);
                else
                    item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j), GROUP_DATETIME_ROLE), GROUP_DATETIME_ROLE);
                item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j), LOG_UID_ROLE), LOG_UID_ROLE);

                rowItems.append(item);
            }

            existItem->insertRow(0, rowItems);

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);

            QString personTypeStr;
            if(blackRecogResult[0].personType == 0)
            {
                personTypeStr = StringTable::Str_White;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::green), Qt::BackgroundColorRole);
            }
            else if(blackRecogResult[0].personType == 1)
            {
                personTypeStr = StringTable::Str_Black;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::red), Qt::BackgroundColorRole);
            }
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 1), personTypeStr);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 2), blackRecogResult[0].areaName);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 3), blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 4), blackRecogResult[0].similiarity);

            QString genderStr;
            if(blackRecogResult[0].gender == 0)
                genderStr = StringTable::Str_Male;
            else
                genderStr = StringTable::Str_Female;

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 5), genderStr);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 6), blackRecogResult[0].birthday);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 7), blackRecogResult[0].address);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 8), blackRecogResult[0].description);

            for(int j = 0; j < 9; j ++)
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, j), blackRecogResult[0].logUID, LOG_UID_ROLE);
        }
        else
        {
            QList<QStandardItem*> rowItems;
            QStandardItem* item = new QStandardItem;
            item->setData(blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            QString personTypeStr;
            if(blackRecogResult[0].personType == 0)
            {
                personTypeStr = StringTable::Str_White;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::green), Qt::BackgroundColorRole);
            }
            else if(blackRecogResult[0].personType == 1)
            {
                personTypeStr = StringTable::Str_Black;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::red), Qt::BackgroundColorRole);
            }

            item = new QStandardItem;
            item->setData(personTypeStr, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].areaName, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"), Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].similiarity, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            QString genderStr;
            if(blackRecogResult[0].gender == 0)
                genderStr = StringTable::Str_Male;
            else
                genderStr = StringTable::Str_Female;

            item = new QStandardItem;
            item->setData(genderStr, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].birthday, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].address, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].description, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            existItem->insertRow(0, rowItems);

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);
        }

        ui->viewMonitoringPageBlackResult->clearSelection();
        ui->viewMonitoringPageBlackResult->setCurrentIndex(m_monitoringPageBlackResultModel->index(selectedRow, 0));
    }
    else
    {
        int rowIndex = 0;
        m_monitoringPageBlackResultModel->insertRow(rowIndex);

        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].name);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].logUID, LOG_UID_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].name, GROUP_NAME_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), m_blackAlarmId, ALRM_ID_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), 1, ALRM_FLAG_ROLE);
        m_blackAlarmId ++;

        QString personTypeStr;
        if(blackRecogResult[0].personType == 0)
        {
            personTypeStr = StringTable::Str_White;
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), QColor(Qt::green), Qt::BackgroundColorRole);
        }
        else if(blackRecogResult[0].personType == 1)
        {
            personTypeStr = StringTable::Str_Black;
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), QColor(Qt::red), Qt::BackgroundColorRole);
        }
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 1), personTypeStr);

        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 2), blackRecogResult[0].areaName);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 3), blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 4), blackRecogResult[0].similiarity);

        QString genderStr;
        if(blackRecogResult[0].gender == 0)
            genderStr = StringTable::Str_Male;
        else
            genderStr = StringTable::Str_Female;

        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 5), genderStr);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 6), blackRecogResult[0].birthday);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 7), blackRecogResult[0].address);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 8), blackRecogResult[0].description);

        for(int j = 0; j < 9; j ++)
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, j), blackRecogResult[0].logUID, LOG_UID_ROLE);

        ui->viewMonitoringPageBlackResult->clearSelection();
        ui->viewMonitoringPageBlackResult->setCurrentIndex(m_monitoringPageBlackResultModel->index(rowIndex, 0));
    }

    if(m_monitoringPageBlackResultModel->rowCount() > N_MAX_BLACK_RECOG_RESULT)
    {
        for(int k = N_MAX_BLACK_RECOG_RESULT; k < m_monitoringPageBlackResultModel->rowCount(); k ++)
        {
            QStandardItem* lastItem = m_monitoringPageBlackResultModel->item(k);
        for(int i = 0; i < lastItem->rowCount(); i ++)
        {
                QStandardItem* childItem = lastItem->child(i);
                QString logUID = childItem->data(LOG_UID_ROLE).toString();

            for(int j = 0; j < m_blackRecogResults.size(); j ++)
            {
                if(m_blackRecogResults[j][0].logUID == logUID)
                {
                    m_blackRecogResults.remove(j);
                    break;
                }
            }
        }

        QString logUID = lastItem->data(LOG_UID_ROLE).toString();
        for(int j = 0; j < m_blackRecogResults.size(); j ++)
        {
            if(m_blackRecogResults[j][0].logUID == logUID)
            {
                m_blackRecogResults.remove(j);
                break;
            }
        }

            m_monitoringPageBlackResultModel->removeRow(k);
            k --;
        }
    }

    showResultAlarmDlg();
}

void MainWindow::slotReceiveLastBlackResult(QVector<BLACK_RECOG_RESULT> blackRecogResult)
{
    for(int i = 0; i < blackRecogResult.size(); i ++)
    {
        int serverIndex = getServerIndexFromUID(m_serverInfoSockets, blackRecogResult[0].serverUID);
        QString areaName = getAreaNameFromUID(m_areaInfos, m_serverInfoSockets, blackRecogResult[0].serverUID, blackRecogResult[0].chanelIndex);
        if(serverIndex < 0)
            continue;

        QString serverInfoStr;
        serverInfoStr.sprintf("%s - %s (%s %d)", areaName.toUtf8().data(),
                              getServerNameFromUID(m_serverInfoSockets, blackRecogResult[0].serverUID).toUtf8().data(),
                              StringTable::Str_Chanel.toUtf8().data(), blackRecogResult[0].chanelIndex + 1);

        blackRecogResult[i].areaName = serverInfoStr;
    }

    for(int i = 0; i < m_blackRecogResults.size(); i ++)
    {
        if(m_blackRecogResults[i][0].logUID == blackRecogResult[0].logUID)
            return;
    }
    m_blackRecogResults.append(blackRecogResult);

    QString addNameStr = blackRecogResult[0].name;
    QDateTime addDateTime = blackRecogResult[0].dateTime;

    QStandardItem* existItem = NULL;
    int selectedRow = -1;
    for(int j = 0; j < m_monitoringPageBlackResultModel->rowCount(); j ++)
    {
        QStandardItem* firstItem = m_monitoringPageBlackResultModel->item(j);
        int diffSec = firstItem->data(GROUP_DATETIME_ROLE).toDateTime().secsTo(addDateTime);
        if(firstItem->data(GROUP_NAME_ROLE).toString() == addNameStr &&
                 diffSec < LAST_LOG_DIFF && diffSec >= 0)
        {
            existItem = firstItem;
            selectedRow = j;
            break;
        }
    }

    if(existItem)
    {
        float oldSimilarity = m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, 4)).toFloat();
        if(oldSimilarity < blackRecogResult[0].similiarity)
        {
            QList<QStandardItem*> rowItems;
            for(int j = 0; j < 9; j ++)
            {
                QStandardItem* item = new QStandardItem;
                if(j != 0)
                    item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j)), Qt::DisplayRole);
                else
                    item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j), GROUP_DATETIME_ROLE), GROUP_DATETIME_ROLE);
                item->setData(m_monitoringPageBlackResultModel->data(m_monitoringPageBlackResultModel->index(selectedRow, j), LOG_UID_ROLE), LOG_UID_ROLE);

                rowItems.append(item);
            }

            existItem->insertRow(0, rowItems);

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);

            QString personTypeStr;
            if(blackRecogResult[0].personType == 0)
            {
                personTypeStr = StringTable::Str_White;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::green), Qt::BackgroundColorRole);
            }
            else if(blackRecogResult[0].personType == 1)
            {
                personTypeStr = StringTable::Str_Black;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::red), Qt::BackgroundColorRole);
            }
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 1), personTypeStr);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 2), blackRecogResult[0].areaName);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 3), blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 4), blackRecogResult[0].similiarity);

            QString genderStr;
            if(blackRecogResult[0].gender == 0)
                genderStr = StringTable::Str_Male;
            else
                genderStr = StringTable::Str_Female;

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 5), genderStr);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 6), blackRecogResult[0].birthday);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 7), blackRecogResult[0].address);
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 8), blackRecogResult[0].description);

            for(int j = 0; j < 9; j ++)
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, j), blackRecogResult[0].logUID, LOG_UID_ROLE);
        }
        else
        {
            QList<QStandardItem*> rowItems;
            QStandardItem* item = new QStandardItem;
            item->setData(blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            QString personTypeStr;
            if(blackRecogResult[0].personType == 0)
            {
                personTypeStr = StringTable::Str_White;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::green), Qt::BackgroundColorRole);
            }
            else if(blackRecogResult[0].personType == 1)
            {
                personTypeStr = StringTable::Str_Black;
                m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), QColor(Qt::red), Qt::BackgroundColorRole);
            }

            item = new QStandardItem;
            item->setData(personTypeStr, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].areaName, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"), Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].similiarity, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            QString genderStr;
            if(blackRecogResult[0].gender == 0)
                genderStr = StringTable::Str_Male;
            else
                genderStr = StringTable::Str_Female;

            item = new QStandardItem;
            item->setData(genderStr, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].birthday, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].address, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            item = new QStandardItem;
            item->setData(blackRecogResult[0].description, Qt::DisplayRole);
            item->setData(blackRecogResult[0].logUID, LOG_UID_ROLE);
            rowItems.append(item);

            existItem->insertRow(0, rowItems);

            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(selectedRow, 0), blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);
        }

        ui->viewMonitoringPageBlackResult->clearSelection();
        ui->viewMonitoringPageBlackResult->setCurrentIndex(m_monitoringPageBlackResultModel->index(selectedRow, 0));
    }
    else
    {
        int rowIndex = 0;
        m_monitoringPageBlackResultModel->insertRow(rowIndex);

        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].name);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].logUID, LOG_UID_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].name, GROUP_NAME_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), blackRecogResult[0].dateTime, GROUP_DATETIME_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), m_blackAlarmId, ALRM_ID_ROLE);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), 0, ALRM_FLAG_ROLE);
        m_blackAlarmId ++;

        QString personTypeStr;
        if(blackRecogResult[0].personType == 0)
        {
            personTypeStr = StringTable::Str_White;
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), QColor(Qt::green), Qt::BackgroundColorRole);
        }
        else if(blackRecogResult[0].personType == 1)
        {
            personTypeStr = StringTable::Str_Black;
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 0), QColor(Qt::red), Qt::BackgroundColorRole);
        }
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 1), personTypeStr);

        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 2), blackRecogResult[0].areaName);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 3), blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 4), blackRecogResult[0].similiarity);

        QString genderStr;
        if(blackRecogResult[0].gender == 0)
            genderStr = StringTable::Str_Male;
        else
            genderStr = StringTable::Str_Female;

        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 5), genderStr);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 6), blackRecogResult[0].birthday);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 7), blackRecogResult[0].address);
        m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, 8), blackRecogResult[0].description);

        for(int j = 0; j < 9; j ++)
            m_monitoringPageBlackResultModel->setData(m_monitoringPageBlackResultModel->index(rowIndex, j), blackRecogResult[0].logUID, LOG_UID_ROLE);

        ui->viewMonitoringPageBlackResult->clearSelection();
        ui->viewMonitoringPageBlackResult->setCurrentIndex(m_monitoringPageBlackResultModel->index(rowIndex, 0));
    }

    if(m_monitoringPageBlackResultModel->rowCount() > N_MAX_BLACK_RECOG_RESULT)
    {
        for(int k = N_MAX_BLACK_RECOG_RESULT; k < m_monitoringPageBlackResultModel->rowCount(); k ++)
        {
            QStandardItem* lastItem = m_monitoringPageBlackResultModel->item(k);
        for(int i = 0; i < lastItem->rowCount(); i ++)
        {
                QStandardItem* childItem = lastItem->child(i);
                QString logUID = childItem->data(LOG_UID_ROLE).toString();

            for(int j = 0; j < m_blackRecogResults.size(); j ++)
            {
                if(m_blackRecogResults[j][0].logUID == logUID)
                {
                    m_blackRecogResults.remove(j);
                    break;
                }
            }
        }

        QString logUID = lastItem->data(LOG_UID_ROLE).toString();
        for(int j = 0; j < m_blackRecogResults.size(); j ++)
        {
            if(m_blackRecogResults[j][0].logUID == logUID)
            {
                m_blackRecogResults.remove(j);
                break;
            }
        }

            m_monitoringPageBlackResultModel->removeRow(k);
            k --;
        }
    }
}

void MainWindow::slotLogItemDoubleClicked(LOG_RESULT logResult)
{
    int exist = -1;
    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        if(m_serverInfoSockets[i]->serverInfo().serverUID == logResult.serverUID)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
        return;

    LogSelectDlg dlg;
    dlg.setInfo(m_areaInfos, m_serverInfoSockets, logResult);
    dlg.exec();

    refreshSelectedBlackInfoByArea();
}

void MainWindow::slotAddServer()
{
    ServerDlg dlg;
    dlg.setServerInfo(m_serverInfoSockets);
    int ret = dlg.exec();
    if(ret == 0)
        return;

    stopSearchLogSockets();
    stopSearchBlackSockets();
    stopSearchLastBlackSockets();
    addServerSocket(dlg.serverInfo());

    refreshServers();
    refreshActions();
    saveSetting();
}

void MainWindow::slotEditServer()
{
    if(!m_selectionModelServerPageServer->hasSelection())
        return;

    QModelIndex index = m_selectionModelServerPageServer->currentIndex();
    int serverIndex = m_modelServerPageServer->data(index, SERVER_INDEX_ROLE).toInt();
    ServerDlg dlg;
    dlg.setEditServerInfo(m_serverInfoSockets, serverIndex);
    int ret = dlg.exec();
    if(ret == 0)
        return;

    stopSearchLogSockets();
    stopSearchBlackSockets();
    stopSearchLastBlackSockets();
    m_serverInfoSockets[serverIndex]->changeServerInfo(dlg.serverInfo());

    refreshServers();
    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();
    saveSetting();
}

void MainWindow::slotDeleteServer()
{
    if(!m_selectionModelServerPageServer->hasSelection())
        return;

    if(QMessageBox::question(this, StringTable::Str_Question, StringTable::Str_Are_you_sure_to_delete_selected_machine ,QMessageBox::Yes | QMessageBox::No , QMessageBox::No) != QMessageBox::Yes)
        return;

    QModelIndex index = m_selectionModelServerPageServer->currentIndex();
    int serverIndex = m_modelServerPageServer->data(index, SERVER_INDEX_ROLE).toInt();

    for(int i = 0; i < m_cameraViewItems.size(); i ++)
    {
        if(m_cameraViewItems[i]->serverInfoSocket() == m_serverInfoSockets[serverIndex])
        {
            m_cameraViewItems[i]->stopMonitoring();
        }
    }

    for(int i = 0; i < m_logImageReceiveSockets.size(); i ++)
    {
        if(m_logImageReceiveSockets[i]->serverInfoSocket() == m_serverInfoSockets[serverIndex])
        {
            m_logImageReceiveSockets[i]->stopSocket();
            delete m_logImageReceiveSockets[i];
            m_logImageReceiveSockets.remove(i);

            m_blackRecogResultReceiveSockets[i]->stopSocket();
            delete m_blackRecogResultReceiveSockets[i];
            m_blackRecogResultReceiveSockets.remove(i);

            i --;

        }
    }

    stopSearchLogSockets();
    stopSearchBlackSockets();
    stopSearchLastBlackSockets();

    m_serverInfoSockets[serverIndex]->stopSocket();
    delete m_serverInfoSockets[serverIndex];
    m_serverInfoSockets.remove(serverIndex);

    for(int i = 0; i < m_areaInfos.size(); i ++)
    {
        for(int j = 0; j < m_areaInfos[i].serverIndexs.size(); j ++)
        {
            if(m_areaInfos[i].serverIndexs[j] == serverIndex)
            {
                m_areaInfos[i].serverIndexs.remove(j);
                m_areaInfos[i].chanelIndexs.remove(j);
                j --;
            }
            else if(m_areaInfos[i].serverIndexs[j] > serverIndex)
                m_areaInfos[i].serverIndexs[j] = m_areaInfos[i].serverIndexs[j] - 1;
        }
    }

    refreshServers();
    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();
    saveSetting();
}

void MainWindow::slotEditIpCamera()
{
    if(!m_selectionModelServerPageServer->hasSelection())
        return;

    QModelIndex index = m_selectionModelServerPageServer->currentIndex();
    int serverIndex = m_modelServerPageServer->data(index, SERVER_INDEX_ROLE).toInt();


    if(serverIndex < 0 || serverIndex >= m_serverInfoSockets.size() || m_serverInfoSockets[serverIndex]->status() == SERVER_STOP)
        return;

    ServerInfo serverInfo = m_serverInfoSockets[serverIndex]->serverInfo();

    IpCameraSettingDlg dlg;
    dlg.setServerInfo(serverInfo);
    dlg.exec();

    QVector<IpCameraInfo> ipCameraInfos = dlg.ipCameraInfos();
    m_serverInfoSockets[serverIndex]->setIpCameraInfos(ipCameraInfos);

    refreshActions();
}

void MainWindow::slotServerStatusChanged(int serverIndex, int status)
{
    refreshServers();
    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();

    if(m_firstStart == 0)
    {
        QVector<int> serverIndexs;
        QVector<int> chanelIndexs;
        for(int i = 0; i < m_areaInfos.size(); i ++)
        {
            serverIndexs << m_areaInfos[i].serverIndexs;
            chanelIndexs << m_areaInfos[i].chanelIndexs;
        }

        m_searchLastBlackSocket = new SearchLastBlackSocket;
        connect(m_searchLastBlackSocket, SIGNAL(receivedLastBlackResults(QVector<BLACK_RECOG_RESULT>)), this, SLOT(slotReceiveLastBlackResult(QVector<BLACK_RECOG_RESULT>)));
        m_searchLastBlackSocket->startSocket(serverIndexs, chanelIndexs, m_serverInfoSockets);

        m_firstStart = 1;
    }
}

void MainWindow::slotServerChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus)
{
    for(int i = 0; i < m_monitoringPageAreaModel->rowCount(); i ++)
    {
        QStandardItem* monitoringPageAreaItem = m_monitoringPageAreaModel->item(i);
        QStandardItem* areaPageAreaItem = m_modelAreaPageArea->item(i);
        QStandardItem* blackPageAreaItem = m_modelBlackPageArea->item(i);
        QStandardItem* searchPageLogAreaItem = m_modelSearchPageLogArea->item(i);
        QStandardItem* searchPageBlackAreaItem = m_modelSearchPageBlackArea->item(i);
        for(int j = 0; j < monitoringPageAreaItem->rowCount(); j ++)
        {
            QStandardItem* monitoringPageChanelItem = monitoringPageAreaItem->child(j);
            QStandardItem* areaPageChanelItem = areaPageAreaItem->child(j);
            QStandardItem* blackPageChanelItem = blackPageAreaItem->child(j);
            QStandardItem* searchPageLogChanelItem = searchPageLogAreaItem->child(j);
            QStandardItem* searchPageBlackChanelItem = searchPageBlackAreaItem->child(j);

            if(monitoringPageChanelItem->data(SERVER_INDEX_ROLE).toInt() == serverIndex &&
                    monitoringPageChanelItem->data(CHANEL_INDEX_ROLE).toInt() == chanelIndex)
            {
                int serverStatus = m_serverInfoSockets[serverIndex]->status();

                if(chanelStatus == CHANEL_STATUS_STOP)
                {
                    if(serverStatus == SERVER_STOP)
                    {
                        monitoringPageChanelItem->setEnabled(false);
                        blackPageChanelItem->setEnabled(false);
                        searchPageLogChanelItem->setEnabled(false);
                        searchPageBlackChanelItem->setEnabled(false);

                    }
                    else
                    {
                        monitoringPageChanelItem->setEnabled(true);
                        blackPageChanelItem->setEnabled(true);
                        searchPageLogChanelItem->setEnabled(true);
                        searchPageBlackChanelItem->setEnabled(true);
                    }
                    monitoringPageChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
                    blackPageChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));

                    areaPageChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
                    searchPageLogChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
                    searchPageBlackChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
                }
                else
                {
                    monitoringPageChanelItem->setEnabled(true);
                    monitoringPageChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

                    blackPageChanelItem->setEnabled(true);
                    blackPageChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

                    searchPageLogChanelItem->setEnabled(true);
                    searchPageLogChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

                    searchPageBlackChanelItem->setEnabled(true);
                    searchPageBlackChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

                    areaPageChanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

                }
            }
        }
    }

    QStandardItem* serverItem = m_modelAreaPageServer->item(serverIndex);
    if(serverItem)
    {
        QStandardItem* chanelItem = serverItem->child(chanelIndex);
        if(chanelItem == NULL)
            return;

        if(chanelStatus == CHANEL_STATUS_STOP)
            chanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
        else
            chanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));
    }
}

void MainWindow::slotBlackInfoChanged(int serverIndex, int chanelIndex)
{
    for(int i = 0; i < m_modelBlackPageArea->rowCount(); i ++)
    {
        QStandardItem* areamItem = m_modelBlackPageArea->item(i);
        for(int j = 0; j < areamItem->rowCount(); j ++)
        {
            QStandardItem* chanelItem = areamItem->child(j);
            if((chanelItem->data(SERVER_INDEX_ROLE).toInt() - 1) == serverIndex &&
                    (chanelItem->data(CHANEL_INDEX_ROLE).toInt() - 1) == chanelIndex)
            {
                int serverStatus = m_serverInfoSockets[serverIndex]->status();
                int chanelStatus = m_serverInfoSockets[serverIndex]->getChanelStatus(chanelIndex);

                ServerInfo serverInfo = m_serverInfoSockets[serverIndex]->serverInfo();
                QString serverInfoName = serverInfo.serverName;
                QString cameraItemText = serverInfoName + " (" + StringTable::Str_Chanel + QString::number(chanelIndex + 1) + ")";
                if(chanelStatus == CHANEL_STATUS_STOP)
                {
                    if(serverStatus == SERVER_STOP)
                        chanelItem->setEnabled(false);

                    chanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
                }
                else
                    chanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

                if(serverStatus != SERVER_STOP)
                    cameraItemText += " - " + QString::number(serverInfo.ipCameraInfos[chanelIndex].blackInfoSize);

                chanelItem->setText(cameraItemText);
            }
        }
    }
}

void MainWindow::slotServerPageServerItemDoubleClicked(QModelIndex)
{
    slotEditServer();
}

void MainWindow::slotServerPageServerItemSelectionChanged(QItemSelection,QItemSelection)
{
    refreshActions();
}


void MainWindow::slotAddArea()
{
    MonitoringAreaDlg dlg;
    dlg.setMonitoringAreaInfos(m_areaInfos);
    int ret = dlg.exec();

    if(ret == 0)
        return;

    m_areaInfos.append(dlg.monitoringAreaInfo());

    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();
    saveSetting();
}

void MainWindow::slotEditArea()
{
    int selectedIndex = selectedEditAreaIndex();
    if(selectedIndex < 0)
    {
        refreshAreaPageArea();
        return;
    }

    MonitoringAreaDlg dlg;
    dlg.setMonitoringAreaInfos(m_areaInfos, selectedIndex);
    int ret = dlg.exec();

    if(ret == 0)
    {
        refreshAreaPageArea();
        return;
    }

    m_areaInfos[selectedIndex].areaName = dlg.monitoringAreaInfo().areaName;

    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();
    saveSetting();
    stopSearchBlackSockets();
    stopSearchLastBlackSockets();
}

void MainWindow::slotDeleteArea()
{
    int selectedIndex = selectedEditAreaIndex();
    if(selectedIndex < 0 || selectedIndex >= m_areaInfos.size())
        return;

    if(QMessageBox::question(this, StringTable::Str_Question, StringTable::Str_Are_you_sure_to_delete_selected_monitoring_area ,QMessageBox::Yes | QMessageBox::No , QMessageBox::No) != QMessageBox::Yes)
        return;

    QStandardItem* areaItem = m_modelAreaPageArea->item(selectedIndex);
    for(int j = areaItem->rowCount() - 1; j >= 0; j --)
    {
        int selectedServerIndex = m_areaInfos[selectedIndex].serverIndexs[j];
        int exist = -1;
        for(int k = 0; k < m_logImageReceiveSockets.size(); k ++)
        {
            if(m_logImageReceiveSockets[k]->serverInfoSocket() == m_serverInfoSockets[selectedServerIndex] &&
                    m_logImageReceiveSockets[k]->chanelIndex() == m_areaInfos[selectedIndex].chanelIndexs[j])
            {
                exist = k;
                break;
            }
        }

        if(exist >= 0)
        {
            m_logImageReceiveSockets[exist]->stopSocket();
            delete m_logImageReceiveSockets[exist];
            m_logImageReceiveSockets.remove(exist);

            m_blackRecogResultReceiveSockets[exist]->stopSocket();
            delete m_blackRecogResultReceiveSockets[exist];
            m_blackRecogResultReceiveSockets.remove(exist);
        }

        for(int k = 0; k < m_cameraViewItems.size(); k ++)
        {
            if(m_cameraViewItems[k]->serverInfoSocket() == m_serverInfoSockets[selectedServerIndex] &&
                    m_cameraViewItems[k]->chanelIndex() == m_areaInfos[selectedIndex].chanelIndexs[j])
            {
                m_cameraViewItems[k]->stopMonitoring();
            }
        }

        m_areaInfos[selectedIndex].chanelIndexs.remove(j);
        m_areaInfos[selectedIndex].serverIndexs.remove(j);
    }

    m_areaInfos.remove(selectedIndex);

    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();
    saveSetting();
    stopSearchBlackSockets();
    stopSearchLastBlackSockets();
}

void MainWindow::slotAddBlackFromSingleFile()
{
    if(m_selectedServerIndexOfEditBlack < 0 || m_selectedChanelIndexOfEditBlack < 0)
        return;

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString fileName = QFileDialog::getOpenFileName(this, StringTable::Str_Open_File, setting.value("last enroll file dir").toString(),
                                                    tr("Image Files(*.bmp *.jpg *.jpeg *.tif *.tiff)"));
    if(fileName.isEmpty())
        return;

    QFileInfo info(fileName);
    setting.setValue("last enroll file dir", info.absoluteDir().absolutePath());

    QImage fileImage(fileName);
    if(fileImage.isNull())
        return;

    AddBlackListFromCapturedDlg dlg;
    dlg.setInfo(m_areaInfos, m_serverInfoSockets, m_selectedServerIndexOfEditBlack, m_selectedChanelIndexOfEditBlack, fileImage, 1);
    dlg.exec();

    refreshSelectedBlackInfoByArea();
    refreshActions();
}

void MainWindow::slotAddBlackFromBatchFile()
{
    if(m_selectedServerIndexOfEditBlack < 0 || m_selectedChanelIndexOfEditBlack < 0)
        return;

    BlackEnrollBatchDlg dlg;
    dlg.setInfo(m_serverInfoSockets[m_selectedServerIndexOfEditBlack], m_selectedChanelIndexOfEditBlack);
    int ret = dlg.exec();
    if(ret)
    {
        QVector<BLACK_PERSON> blackPersonInfo = dlg.blackPersonInfos();

        stopAddBlackInfoSocket();

        m_addBlackInfoSocket = new AddBlackInfoSocket;
        m_addBlackInfoSocket->setInfo(m_serverInfoSockets[m_selectedServerIndexOfEditBlack]->serverInfo(), m_selectedChanelIndexOfEditBlack, blackPersonInfo);

        connect(m_addBlackInfoSocket, SIGNAL(enrollFinished(QObject*)), this, SLOT(slotAddBlackInfoSocketFinished(QObject*)));

        m_addBlackInfoSocket->startSocket();

//        m_progressDialog->setMaximum(0);
//        m_progressDialog->setValue(0);
//        m_progressDialog->show();
    }
    refreshActions();
}

void MainWindow::slotEditBlackPerson()
{
    if(m_selectedServerIndexOfEditBlack < 0 || m_selectedChanelIndexOfEditBlack < 0)
        return;

    if(!m_selectionModelBlackPageBlack->hasSelection())
        return;

    QModelIndex selectedIndex = m_selectionModelBlackPageBlack->selection().indexes().at(0);
    QString selectedName = m_modelBlackPageBlack->data(m_modelBlackPageBlack->index(selectedIndex.row(), 0)).toString();

    int exist = -1;
    for(int i = 0; i < m_selectedBlackMetaInfo.size(); i ++)
    {
        if(m_selectedBlackMetaInfo[i].name == selectedName)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
        return;

    EditBlackInfoDlg editBlackInfoDlg;
    editBlackInfoDlg.setInfo(m_serverInfoSockets[m_selectedServerIndexOfEditBlack], m_selectedChanelIndexOfEditBlack, m_selectedBlackMetaInfo[exist]);

    int ret = editBlackInfoDlg.exec();
    if(ret)
    {
        BLACK_PERSON blackPersonInfo = editBlackInfoDlg.blackPersonInfo();

        stopEditBlackInfoSocket();

        m_editBlackInfoSocket = new EditOneBlackListSocket;
        m_editBlackInfoSocket->setInfo(m_serverInfoSockets[m_selectedServerIndexOfEditBlack]->serverInfo(), m_selectedChanelIndexOfEditBlack, blackPersonInfo, m_selectedBlackMetaInfo[exist].name);

        connect(m_editBlackInfoSocket, SIGNAL(editFinished(QObject*)), this, SLOT(slotEditBlackInfoSocketFinished(QObject*)));

        m_editBlackInfoSocket->startSocket();
    }
    refreshActions();
}

void MainWindow::slotDeleteBlackPerson()
{
    QStringList deleteInfoNames;
    QModelIndexList selectedIndexs = m_selectionModelBlackPageBlack->selectedIndexes();
    for(int i = 0; i < selectedIndexs.size(); i ++)
    {
        QString selectedName = m_modelBlackPageBlack->data(m_modelBlackPageBlack->index(selectedIndexs[i].row(), 0)).toString();
        int exist = -1;
        for(int j = 0; j < deleteInfoNames.size(); j ++)
        {
            if(deleteInfoNames[j] == selectedName)
            {
                exist = j;
                break;
            }
        }

        if(exist < 0)
            deleteInfoNames.append(selectedName);
    }

    if(deleteInfoNames.size() < 0 || m_selectedServerIndexOfEditBlack < 0 || m_selectedChanelIndexOfEditBlack < 0)
        return;

    if(QMessageBox::question(this, StringTable::Str_Question, StringTable::Str_Are_you_sure_to_delete_selected_black_person_infos ,QMessageBox::Yes | QMessageBox::No , QMessageBox::No) != QMessageBox::Yes)
        return;

    stopBlackInfoDelete();

    m_deletBlackListSocket = new DeleteBlackListSocket;
    m_deletBlackListSocket->setInfo(m_serverInfoSockets[m_selectedServerIndexOfEditBlack]->serverInfo(), m_selectedChanelIndexOfEditBlack, deleteInfoNames);

    connect(m_deletBlackListSocket, SIGNAL(deleteFinished(QObject*)), this, SLOT(slotDeleteBlackInfoSocketFinished(QObject*)));

    m_deletBlackListSocket->startDelete();
    refreshActions();
}


void MainWindow::slotAddCameraToArea()
{
    if(m_areaInfos.size() == 0)
        return;

    AreaSelectDlg dlg;
    dlg.setAreaInfo(m_areaInfos);
    int ret = dlg.exec();
    if(ret == 0)
        return;

    int selectedAreaIndex = dlg.selectedIndex();
    if(selectedAreaIndex < 0)
        return;

    for(int i = 0; i < m_modelAreaPageServer->rowCount(); i ++)
    {
        QStandardItem* serverItem = m_modelAreaPageServer->item(i);
        for(int j = 0; j < serverItem->rowCount(); j ++)
        {
            QStandardItem* serverCameraItem = serverItem->child(j);
            if(serverCameraItem->data(Qt::CheckStateRole) == Qt::Checked)
            {
                int exist = 0;

                for(int a = 0; a < m_areaInfos.size(); a ++)
                {
                    for(int b = 0; b < m_areaInfos[a].serverIndexs.size(); b ++)
                    {
                        if(m_areaInfos[a].serverIndexs[b] == i &&
                                m_areaInfos[a].chanelIndexs[b] == j)
                            exist = 1;
                    }
                }

                if(exist == 0)
                {
                    addLogFaceReceiveSocket(i, j);
                    addBlackRecogResultReceiveSocket(i, j);

                    m_areaInfos[selectedAreaIndex].serverIndexs.append(i);
                    m_areaInfos[selectedAreaIndex].chanelIndexs.append(j);
                }

                serverCameraItem->setData(Qt::Unchecked, Qt::CheckStateRole);
            }
        }
    }

    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();
    saveSetting();
}

void MainWindow::slotRemoveCameraFromArea()
{
    for(int i = m_modelAreaPageArea->rowCount() - 1; i >= 0; i --)
    {
        QStandardItem* areaItem = m_modelAreaPageArea->item(i);
        for(int j = areaItem->rowCount() - 1; j >= 0; j --)
        {
            QStandardItem* areaCameraItem = areaItem->child(j);
            if(areaCameraItem->data(Qt::CheckStateRole) == Qt::Checked)
            {
                int selectedServerIndex = m_areaInfos[i].serverIndexs[j];
                int exist = -1;
                for(int k = 0; k < m_logImageReceiveSockets.size(); k ++)
                {
                    if(m_logImageReceiveSockets[k]->serverInfoSocket() == m_serverInfoSockets[selectedServerIndex] &&
                            m_logImageReceiveSockets[k]->chanelIndex() == m_areaInfos[i].chanelIndexs[j])
                    {
                        exist = k;
                        break;
                    }
                }

                if(exist >= 0)
                {
                    m_logImageReceiveSockets[exist]->stopSocket();
                    delete m_logImageReceiveSockets[exist];
                    m_logImageReceiveSockets.remove(exist);

                    m_blackRecogResultReceiveSockets[exist]->stopSocket();
                    delete m_blackRecogResultReceiveSockets[exist];
                    m_blackRecogResultReceiveSockets.remove(exist);
                }

                for(int k = 0; k < m_cameraViewItems.size(); k ++)
                {
                    if(m_cameraViewItems[k]->serverInfoSocket() == m_serverInfoSockets[selectedServerIndex] &&
                            m_cameraViewItems[k]->chanelIndex() == m_areaInfos[i].chanelIndexs[j])
                    {
                        m_cameraViewItems[k]->stopMonitoring();
                    }
                }

                m_areaInfos[i].chanelIndexs.remove(j);
                m_areaInfos[i].serverIndexs.remove(j);
            }
        }
    }

    refreshMonitoringPageArea();
    refreshAreaPageArea();
    refreshBlackPageArea();
    refreshSearchPageArea();
    refreshActions();
    saveSetting();
}

void MainWindow::slotAreaEditItemDoubleClicked(QModelIndex index)
{
    QStandardItem* standardItem = m_modelAreaPageArea->itemFromIndex(index);
    if(standardItem->parent() == NULL)
        slotEditArea();
    else
    {
        int serverIndex = -1, chanelIndex = -1;
        for(int i = 0; i < m_serverInfoSockets.size(); i ++)
        {
            ServerInfo serverInfo = m_serverInfoSockets[i]->serverInfo();
            for(int j = 0; j < serverInfo.ipCameraInfos.size(); j ++)
            {
                QString cameraItemText = serverInfo.serverName + " (" + StringTable::Str_Chanel + QString::number(j + 1) + ")";
                if(cameraItemText == index.data().toString())
                {
                    serverIndex = i;
                    chanelIndex = j;
                }
            }
        }

        if(serverIndex >= 0 && chanelIndex >= 0)
            editSelectedIpCamera(serverIndex, chanelIndex);
    }
}

void MainWindow::slotAreaServerItemDoubleClicked(QModelIndex index)
{
    return;
    QStandardItem* standardItem = m_modelAreaPageServer->itemFromIndex(index);
    if(standardItem->parent() == NULL)
    {
        int exist = -1;
        for(int i = 0; i < m_serverInfoSockets.size(); i ++)
        {
            ServerInfo serverInfo = m_serverInfoSockets[i]->serverInfo();
            if(serverInfo.serverName == index.data().toString())
            {
                exist = i;
                break;
            }
        }

        if(exist < 0)
        {
            refreshServers();
            return;
        }

        ServerInfo serverInfo = m_serverInfoSockets[exist]->serverInfo();

        IpCameraSettingDlg dlg;
        dlg.setServerInfo(serverInfo);
        dlg.exec();

        QVector<IpCameraInfo> ipCameraInfos = dlg.ipCameraInfos();
        m_serverInfoSockets[exist]->setIpCameraInfos(ipCameraInfos);

        refreshServers();
    }
    else
    {
        QStandardItem* parentItem = standardItem->parent();

        int serverIndex = -1, chanelIndex = -1;
        for(int i = 0; i < m_serverInfoSockets.size(); i ++)
        {
            ServerInfo serverInfo = m_serverInfoSockets[i]->serverInfo();
            if(serverInfo.serverName == parentItem->text())
            {
                for(int j = 0; j < serverInfo.ipCameraInfos.size(); j ++)
                {
                    QString chanelIndexStr;
                    chanelIndexStr.sprintf(" (%d)", j + 1);
                    if(index.data() == StringTable::Str_Chanel + chanelIndexStr)
                    {
                        serverIndex = i;
                        chanelIndex = j;
                    }
                }
            }
        }

        if(serverIndex >= 0 && chanelIndex >= 0)
            editSelectedIpCamera(serverIndex, chanelIndex);
    }
}

void MainWindow::slotAreaEditItemDataChanged(QModelIndex topLeft, QModelIndex topRight, QVector<int> roles)
{
    QStandardItem* standardItem = m_modelAreaPageArea->itemFromIndex(topLeft);
    if(standardItem == NULL)
        return;

    if(standardItem->hasChildren())
    {
        for(int i = 0; i < standardItem->rowCount(); i ++)
            standardItem->child(i)->setCheckState(standardItem->checkState());
    }
    else if(standardItem->parent())
    {
        if(standardItem->checkState() == Qt::Unchecked)
        {
            int isChecked = 0;
            QStandardItem* parentItem = standardItem->parent();
            for(int i = 0; i < parentItem->rowCount(); i ++)
            {
                if(parentItem->child(i)->checkState() != Qt::Unchecked)
                    isChecked = 1;
            }

            if(isChecked == 0)
                parentItem->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::slotAreaServerItemDataChanged(QModelIndex topLeft, QModelIndex topRight, QVector<int> roles)
{
    QStandardItem* standardItem = m_modelAreaPageServer->itemFromIndex(topLeft);
    if(standardItem == NULL)
        return;

    if(standardItem->hasChildren())
    {
        for(int i = 0; i < standardItem->rowCount(); i ++)
            standardItem->child(i)->setCheckState(standardItem->checkState());
    }
    else if(standardItem->parent())
    {
        if(standardItem->checkState() == Qt::Unchecked)
        {
            int isChecked = 0;
            QStandardItem* parentItem = standardItem->parent();
            for(int i = 0; i < parentItem->rowCount(); i ++)
            {
                if(parentItem->child(i)->checkState() != Qt::Unchecked)
                    isChecked = 1;
            }

            if(isChecked == 0)
                parentItem->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::slotAreaPageAreaItemSelectionChaned(QItemSelection, QItemSelection)
{
    refreshActions();
}

void MainWindow::slotEditBlackAreaSelectionChanged(QItemSelection ,QItemSelection)
{
    refreshSelectedBlackInfoByArea();
}

void MainWindow::slotReceivedSelectedBlackInfos(QVector<BLACK_PERSON> blackInfos)
{
    m_selectedBlackMetaInfo = blackInfos;
    refreshSelectedBlackMetaInfos();

    ui->progressBarBlack->hide();
//    QApplication::restoreOverrideCursor();

    refreshActions();
}

void MainWindow::slotBlackInfoItemDoubleClicked(QModelIndex)
{
    slotEditBlackPerson();
}

void MainWindow::slotBlackInfoItemSelectionChanged(QItemSelection, QItemSelection)
{
    refreshActions();
    if(m_selectedServerIndexOfEditBlack < 0 || m_selectedChanelIndexOfEditBlack < 0)
    {
        ui->lblBlackInfoFace->setPixmap(QPixmap());
        return;
    }

    if(!m_selectionModelBlackPageBlack->hasSelection())
    {
        ui->lblBlackInfoFace->setPixmap(QPixmap());
        return;
    }

    QModelIndex selectedIndex = m_selectionModelBlackPageBlack->selection().indexes().at(0);
    QString selectedName = m_modelBlackPageBlack->data(m_modelBlackPageBlack->index(selectedIndex.row(), 0)).toString();

    int exist = -1;
    for(int i = 0; i < m_selectedBlackMetaInfo.size(); i ++)
    {
        if(m_selectedBlackMetaInfo[i].name == selectedName)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
    {
        ui->lblBlackInfoFace->setPixmap(QPixmap());
        return;
    }

    ui->lblBlackInfoFace->setPixmap(QPixmap::fromImage(qByteArray2Image(m_selectedBlackMetaInfo[exist].faceData[0])));
}

void MainWindow::slotDeleteBlackInfoSocketFinished(QObject*)
{
    stopBlackInfoDelete();
    refreshSelectedBlackInfoByArea();
}

void MainWindow::slotAddBlackInfoSocketFinished(QObject*)
{
//    m_progressDialog->close();
    stopAddBlackInfoSocket();
    refreshSelectedBlackInfoByArea();
}

void MainWindow::slotEditBlackInfoSocketFinished(QObject*)
{
    stopEditBlackInfoSocket();
    refreshSelectedBlackInfoByArea();
}


void MainWindow::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* e)
{
    if(obj == ui->viewMonitoringPageCamera && e->type() == QEvent::Resize)
    {
        if(m_sceneMonitoringPageCamera)
        {
            relocateCameraViewItems();
            return false;
        }
    }
    else if(obj == ui->viewMonitoringPageLogResult && e->type() == QEvent::Resize)
    {
        if(m_sceneMonitoringPageLog)
        {
            relocateLogViewItems();
            return false;
        }
    }
    else if(obj == ui->viewSeachPageLogResult && e->type() == QEvent::Resize)
    {
        if(m_sceneSearchPageLogResult)
        {
            relocateSearchLogResults();
            return false;
        }
    }
    return false;
}

void MainWindow::timerEvent(QTimerEvent* e)
{
    if(e->timerId() == m_timerID)
        saveLog();
}


void MainWindow::slotViewSearchLog()
{
    ui->uiSearchPageManager->setCurrentIndex(0);

    ui->dateTimeSearchLogStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchLogEnd->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeSearchBlackStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchBlackEnd->setDateTime(QDateTime::currentDateTime());


    ui->actionSearchPageLog->setChecked(true);
    ui->actionSearchPageBlack->setChecked(false);
}

void MainWindow::slotViewSearchBlack()
{
    ui->uiSearchPageManager->setCurrentIndex(1);

    ui->dateTimeSearchLogStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchLogEnd->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeSearchBlackStart->setDateTime(QDateTime(QDate::currentDate(), QTime()));
    ui->dateTimeSearchBlackEnd->setDateTime(QDateTime::currentDateTime());


    ui->actionSearchPageLog->setChecked(false);
    ui->actionSearchPageBlack->setChecked(true);
}


void MainWindow::slotSearchLog()
{
    if(m_searchLogSocket)
    {
        stopSearchLogSockets();
    }
    else
    {
        QVector<int> selectedChanels;
        QVector<int> selectedServers;
        QVector<QString> selectedAreaNames;

        for(int i = 0; i < m_modelSearchPageLogArea->rowCount(); i ++)
        {
            QStandardItem* areaItem = m_modelSearchPageLogArea->item(i);
            for(int j = 0; j < areaItem->rowCount(); j ++)
            {
                QStandardItem* chanelItem = areaItem->child(j);
                if(chanelItem->isEnabled() && chanelItem->checkState() == Qt::Checked)
                {
                    int serverIndex = chanelItem->data(SERVER_INDEX_ROLE).toInt();
                    int chanelIndex = chanelItem->data(CHANEL_INDEX_ROLE).toInt();

                    QString areaName = getAreaNameFromUID(m_areaInfos, m_serverInfoSockets, m_serverInfoSockets[serverIndex]->serverInfo().serverUID, chanelIndex);

                    selectedServers.append(serverIndex);
                    selectedChanels.append(chanelIndex);
                    selectedAreaNames.append(areaName);
                }
            }
        }

        if(selectedServers.size() == 0)
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_select_chanel);
            return;
        }
        else
        {
            m_sceneSearchPageLogResult->clear();
            m_searchPageLogResuttItems.clear();

            m_searchLogCount = 0;
            m_searchCurLogPageIndex = 0;
            m_searchAllLogPageCount = 0;
            refreshLogNavigation();

            m_searchLogSocket = new SearchLogSocket;
            connect(m_searchLogSocket, SIGNAL(socketFinished(QObject*)), this, SLOT(slotSearchLogSocketFinished(QObject*)));
            connect(m_searchLogSocket, SIGNAL(receivedLogResult(LOG_RESULT)), this, SLOT(slotReceiveSearchLogResult(LOG_RESULT)));

            m_searchLogSocket->startSocket(selectedServers, selectedChanels, selectedAreaNames, m_serverInfoSockets,
                                           ui->dateTimeSearchLogStart->dateTime(), ui->dateTimeSearchLogEnd->dateTime(),
                                           m_searchLogFaceInfo.featData, ui->sldThreshold->value());

            ui->progressSearchLog->show();
            ui->btnSearchLog->setText(StringTable::Str_Stop);
        }
    }
}


void MainWindow::slotReceiveSearchLogResult(LOG_RESULT logResult)
{
    m_searchLogCount ++;
    refreshLogNavigation();
}

void MainWindow::slotFirstLog()
{
    if(m_searchCurLogPageIndex == 0)
        return;

    m_searchCurLogPageIndex = 0;
    refreshCurLog();
}

void MainWindow::slotBackwardLog()
{
    if(m_searchCurLogPageIndex == 0)
        return;

    m_searchCurLogPageIndex -= 5;
    if(m_searchCurLogPageIndex < 0)
        m_searchCurLogPageIndex = 0;

    refreshCurLog();
}

void MainWindow::slotPrevLog()
{
    if(m_searchCurLogPageIndex == 0)
        return;

    m_searchCurLogPageIndex --;
    if(m_searchCurLogPageIndex < 0)
        m_searchCurLogPageIndex = 0;

    refreshCurLog();
}

void MainWindow::slotNextLog()
{
    if(m_searchCurLogPageIndex == m_searchAllLogPageCount - 1)
        return;

    m_searchCurLogPageIndex ++;
    if(m_searchCurLogPageIndex >= m_searchAllLogPageCount)
        m_searchCurLogPageIndex = m_searchAllLogPageCount - 1;

    refreshCurLog();
}

void MainWindow::slotForwardLog()
{
    if(m_searchCurLogPageIndex == m_searchAllLogPageCount - 1)
        return;

    m_searchCurLogPageIndex += 5;
    if(m_searchCurLogPageIndex >= m_searchAllLogPageCount)
        m_searchCurLogPageIndex = m_searchAllLogPageCount - 1;

    refreshCurLog();
}

void MainWindow::slotLastLog()
{
    if(m_searchCurLogPageIndex == m_searchAllLogPageCount - 1)
        return;

    m_searchCurLogPageIndex = m_searchAllLogPageCount - 1;
    refreshCurLog();
}

void MainWindow::refreshLogNavigation()
{
    m_searchAllLogPageCount = (m_searchLogCount + N_SEARCH_LOG_PAGE - 1) / 100;
    ui->editAllLog->setText(QString::number(m_searchAllLogPageCount));
    if(m_searchAllLogPageCount == 0)
    {
        ui->btnLogFirst->hide();
        ui->btnLogBackward->hide();
        ui->btnLogPrev->hide();
        ui->btnLogNext->hide();
        ui->btnLogForward->hide();
        ui->btnLogLast->hide();
        ui->editCurLog->hide();
        ui->editAllLog->hide();
        ui->lblSearchLogSep->hide();
    }
    else
    {
        ui->btnLogFirst->show();
        ui->btnLogBackward->show();
        ui->btnLogPrev->show();
        ui->btnLogNext->show();
        ui->btnLogForward->show();
        ui->btnLogLast->show();
        ui->editCurLog->show();
        ui->editAllLog->show();
        ui->lblSearchLogSep->show();
    }

    if((m_searchCurLogPageIndex + 1) == m_searchAllLogPageCount)
        refreshCurLog();
}

void MainWindow::refreshCurLog()
{
    m_sceneSearchPageLogResult->clear();
    m_searchPageLogResuttItems.clear();

    if(m_searchCurLogPageIndex < 0 || m_searchCurLogPageIndex >= m_searchAllLogPageCount)
        return;

    ui->editCurLog->setText(QString::number(m_searchCurLogPageIndex + 1));

    int startIndex = m_searchCurLogPageIndex * N_SEARCH_LOG_PAGE;
    int endIndex = qMin((m_searchCurLogPageIndex + 1) * N_SEARCH_LOG_PAGE, m_searchLogCount);

    for(int i = startIndex; i < endIndex; i ++)
    {
        LOG_RESULT faceImageInfo;
        QByteArray cacheData;
        cacheData.resize(CACHE_LOG_SIZE);
        FILE* fp = fopen(CACHE_LOG_NAME, "rb");
        if(!fp)
            return;

        fseek(fp, CACHE_LOG_SIZE * i, SEEK_SET);
        fread(cacheData.data(), cacheData.size(), 1, fp);
        fclose(fp);

        QBuffer cacheBuffer(&cacheData);
        cacheBuffer.open(QIODevice::ReadOnly);
        QDataStream cacheStream(&cacheBuffer);

        cacheStream >> faceImageInfo.serverUID;
        cacheStream >> faceImageInfo.chanelIndex;
        cacheStream >> faceImageInfo.logUID;
        cacheStream >> faceImageInfo.dateTime;
        cacheStream >> faceImageInfo.faceImage;
        cacheStream >> faceImageInfo.areaName;

        cacheBuffer.close();

        CacheLogViewItem* searchLogItem = new CacheLogViewItem;
        searchLogItem->setInfo(faceImageInfo);
        connect(searchLogItem, SIGNAL(logItemDoubleClicked(LOG_RESULT)), this, SLOT(slotLogItemDoubleClicked(LOG_RESULT)));

        m_sceneSearchPageLogResult->addItem(searchLogItem);
        m_searchPageLogResuttItems.append(searchLogItem);
    }

    relocateSearchLogResults();

}

void MainWindow::slotSearchLogSocketFinished(QObject* obj)
{
    stopSearchLogSockets();
    refreshLogNavigation();
}


void MainWindow::slotSetFace()
{
    if(m_searchLogFaceInfo.faceImage.isNull())
    {
        int exist = -1;
        for(int i = 0; i < m_serverInfoSockets.size(); i ++)
        {
            if(m_serverInfoSockets[i]->status() != SERVER_STOP)
            {
                exist = i;
                break;
            }
        }

        if(exist < 0)
            return;

        BlackFaceAddDlg dlg;
        dlg.setInfo(m_serverInfoSockets[exist], 1);

        if(dlg.exec() == 0)
            return;

        QVector<FRAME_RESULT> enrolledFrameResult = dlg.enrollFrameResults();
        if(enrolledFrameResult.size() == 0)
            return;

        m_searchLogFaceInfo = enrolledFrameResult[0];

        refreshSearchLogFace();

        ui->btnSetFace->setText(StringTable::Str_Delete);
    }
    else
    {
        m_searchLogFaceInfo.faceImage = QImage();
        m_searchLogFaceInfo.featData = QByteArray();
        m_searchLogFaceInfo.faceRect = QRect();

        refreshSearchLogFace();
        ui->btnSetFace->setText(StringTable::Str_Set_Face);
    }
}

void MainWindow::slotThresoldChanged(int value)
{
    ui->lblCurThresold->setText(QString::number(value));
}

void MainWindow::slotSearchPageLogAreaItemDataChanged(QModelIndex topLeft, QModelIndex , QVector<int> )
{
    QStandardItem* standardItem = m_modelSearchPageLogArea->itemFromIndex(topLeft);
    if(standardItem->hasChildren())
    {
        for(int i = 0; i < standardItem->rowCount(); i ++)
        {
            if(standardItem->child(i)->isEnabled())
                standardItem->child(i)->setCheckState(standardItem->checkState());
        }
    }
    else if(standardItem->parent())
    {
        if(standardItem->checkState() == Qt::Unchecked)
        {
            int isChecked = 0;
            QStandardItem* parentItem = standardItem->parent();
            for(int i = 0; i < parentItem->rowCount(); i ++)
            {
                if(parentItem->child(i)->checkState() != Qt::Unchecked)
                    isChecked = 1;
            }

            if(isChecked == 0)
                parentItem->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::slotSearchBlack()
{
    if(m_searchBlackSocket)
    {
        stopSearchBlackSockets();
    }
    else
    {
        QVector<int> selectedServers;
        QVector<int> selectedChanels;
        QVector<QString> selectedAreaNames;

        for(int i = 0; i < m_modelSearchPageBlackArea->rowCount(); i ++)
        {
            QStandardItem* areaItem = m_modelSearchPageBlackArea->item(i);
            for(int j = 0; j < areaItem->rowCount(); j ++)
            {
                QStandardItem* chanelItem = areaItem->child(j);
                if(chanelItem->isEnabled() && chanelItem->checkState() == Qt::Checked)
                {
                    int serverIndex = chanelItem->data(SERVER_INDEX_ROLE).toInt();
                    int chanelIndex = chanelItem->data(CHANEL_INDEX_ROLE).toInt();

                    QString areaName = getAreaNameFromUID(m_areaInfos, m_serverInfoSockets, m_serverInfoSockets[serverIndex]->serverInfo().serverUID, chanelIndex);

                    selectedServers.append(serverIndex);
                    selectedChanels.append(chanelIndex);
                    selectedAreaNames.append(areaName);
                }
            }
        }
        if(selectedServers.size() == 0)
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_select_chanel);
            return;
        }
        else
        {
            m_modelSearchPageBlackList->removeRows(0, m_modelSearchPageBlackList->rowCount());
            ui->widgetSearchBlackResult->setBlackResult(QVector<BLACK_RECOG_RESULT>());

            m_searchBlackCount = 0;
            m_searchCurBlackPageIndex = 0;
            m_searchAllBlackPageCount = 0;
            refreshBlackNavigation();

            m_searchBlackSocket = new SearchBlackSocket;
            connect(m_searchBlackSocket, SIGNAL(socketFinished(QObject*)), this, SLOT(slotSearchBlackSocketFinished(QObject*)));
            connect(m_searchBlackSocket, SIGNAL(receivedBlackRecogResult(QVector<BLACK_RECOG_RESULT>)), this, SLOT(slotSearchBlackRecogResults(QVector<BLACK_RECOG_RESULT>)));

            m_searchBlackSocket->startSocket(selectedServers, selectedChanels, selectedAreaNames, m_serverInfoSockets,
                                           ui->dateTimeSearchBlackStart->dateTime(), ui->dateTimeSearchBlackEnd->dateTime(),
                                           ui->editSearchBlackName->text());

            ui->progressSearchBlack->show();
            ui->btnSearchBlack->setText(StringTable::Str_Stop);
        }
    }
}


void MainWindow::refreshBlackNavigation()
{
    m_searchAllBlackPageCount = (m_searchBlackCount + N_SEARCH_BLACK_PAGE - 1) / 100;
    ui->editAllBlack->setText(QString::number(m_searchAllBlackPageCount));
    if(m_searchAllBlackPageCount == 0)
    {
        ui->btnBlackFirst->hide();
        ui->btnBlackBackward->hide();
        ui->btnBlackPrev->hide();
        ui->btnBlackNext->hide();
        ui->btnBlackForward->hide();
        ui->btnBlackLast->hide();
        ui->editCurBlack->hide();
        ui->editAllBlack->hide();
        ui->lblSearchBlackSep->hide();
    }
    else
    {
        ui->btnBlackFirst->show();
        ui->btnBlackBackward->show();
        ui->btnBlackPrev->show();
        ui->btnBlackNext->show();
        ui->btnBlackForward->show();
        ui->btnBlackLast->show();
        ui->editCurBlack->show();
        ui->editAllBlack->show();
        ui->lblSearchBlackSep->show();
    }

    if((m_searchCurBlackPageIndex + 1) == m_searchAllBlackPageCount)
        refreshCurBlack();
}

void MainWindow::refreshCurBlack()
{
    m_modelSearchPageBlackList->removeRows(0, m_modelSearchPageBlackList->rowCount());

    if(m_searchCurBlackPageIndex < 0 || m_searchCurBlackPageIndex >= m_searchAllBlackPageCount)
        return;

    ui->editCurBlack->setText(QString::number(m_searchCurBlackPageIndex + 1));

    int startIndex = m_searchCurBlackPageIndex * N_SEARCH_BLACK_PAGE;
    int endIndex = qMin((m_searchCurBlackPageIndex + 1) * N_SEARCH_BLACK_PAGE, m_searchBlackCount);

    for(int i = startIndex; i < endIndex; i ++)
    {
        int rowIndex = m_modelSearchPageBlackList->rowCount();
        m_modelSearchPageBlackList->insertRow(rowIndex);

        BLACK_RECOG_RESULT candidateResult;
        QByteArray cacheData;
        cacheData.resize(CACHE_BLACK_SIZE);
        FILE* fp = fopen(CACHE_BLACK_NAME, "rb");
        if(!fp)
            return;

        fseek(fp, CACHE_BLACK_SIZE * i, SEEK_SET);
        fread(cacheData.data(), cacheData.size(), 1, fp);
        fclose(fp);

        QBuffer cacheBuffer(&cacheData);
        cacheBuffer.open(QIODevice::ReadOnly);
        QDataStream cacheStream(&cacheBuffer);

        cacheStream >> candidateResult.name;
        cacheStream >> candidateResult.gender;
        cacheStream >> candidateResult.birthday;
        cacheStream >> candidateResult.address;
        cacheStream >> candidateResult.description;
        cacheStream >> candidateResult.personType;
        cacheStream >> candidateResult.galleryFaceData;
        cacheStream >> candidateResult.probeFaceData;
        cacheStream >> candidateResult.similiarity;
        cacheStream >> candidateResult.areaName;
        cacheStream >> candidateResult.logUID;
        cacheStream >> candidateResult.dateTime;
        cacheStream >> candidateResult.serverUID;
        cacheStream >> candidateResult.chanelIndex;
        cacheBuffer.close();

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 0), candidateResult.name);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 0), Qt::AlignCenter, Qt::TextAlignmentRole);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 0), rowIndex, ROW_INDEX_ROLE);

        QString personTypeStr;
        if(candidateResult.personType == 0)
        {
            personTypeStr = StringTable::Str_White;
            m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 0), QColor(Qt::green), Qt::BackgroundColorRole);
        }
        else if(candidateResult.personType == 1)
        {
            personTypeStr = StringTable::Str_Black;
            m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 0), QColor(Qt::red), Qt::BackgroundColorRole);
        }
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 1), personTypeStr);

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 2), candidateResult.areaName);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 2), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 3), candidateResult.dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 3), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 4), candidateResult.similiarity);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 4), Qt::AlignCenter, Qt::TextAlignmentRole);

        QString genderStr;
        if(candidateResult.gender == 0)
            genderStr = StringTable::Str_Male;
        else
            genderStr = StringTable::Str_Female;

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 5), genderStr);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 5), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 6), candidateResult.birthday);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 6), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 7), candidateResult.address);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 7), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 8), candidateResult.description);
        m_modelSearchPageBlackList->setData(m_modelSearchPageBlackList->index(rowIndex, 8), Qt::AlignCenter, Qt::TextAlignmentRole);
    }
}


void MainWindow::slotFirstBlack()
{
    if(m_searchCurBlackPageIndex == 0)
        return;

    m_searchCurBlackPageIndex = 0;
    refreshCurBlack();
}

void MainWindow::slotBackwardBlack()
{
    if(m_searchCurBlackPageIndex == 0)
        return;

    m_searchCurBlackPageIndex -= 5;
    if(m_searchCurBlackPageIndex < 0)
        m_searchCurBlackPageIndex = 0;

    refreshCurBlack();
}

void MainWindow::slotPrevBlack()
{
    if(m_searchCurBlackPageIndex == 0)
        return;

    m_searchCurBlackPageIndex --;
    if(m_searchCurBlackPageIndex < 0)
        m_searchCurBlackPageIndex = 0;

    refreshCurBlack();
}

void MainWindow::slotNextBlack()
{
    if(m_searchCurBlackPageIndex == m_searchAllBlackPageCount - 1)
        return;

    m_searchCurBlackPageIndex ++;
    if(m_searchCurBlackPageIndex >= m_searchAllBlackPageCount)
        m_searchCurBlackPageIndex = m_searchAllBlackPageCount - 1;

    refreshCurBlack();
}

void MainWindow::slotForwardBlack()
{
    if(m_searchCurBlackPageIndex == m_searchAllBlackPageCount - 1)
        return;

    m_searchCurBlackPageIndex += 5;
    if(m_searchCurBlackPageIndex >= m_searchAllBlackPageCount)
        m_searchCurBlackPageIndex = m_searchAllBlackPageCount - 1;

    refreshCurBlack();
}

void MainWindow::slotLastBlack()
{
    if(m_searchCurBlackPageIndex == m_searchAllBlackPageCount - 1)
        return;

    m_searchCurBlackPageIndex = m_searchAllBlackPageCount - 1;
    refreshCurBlack();
}


void MainWindow::slotSearchBlackRecogResults(QVector<BLACK_RECOG_RESULT> blackRecogResult)
{
    m_searchBlackCount ++;
    refreshBlackNavigation();
}


void MainWindow::slotSearchBlackSocketFinished(QObject* obj)
{
    stopSearchBlackSockets();
}


void MainWindow::slotSearchPageBlackAreaItemDataChanged(QModelIndex topLeft, QModelIndex , QVector<int> )
{
    QStandardItem* standardItem = m_modelSearchPageBlackArea->itemFromIndex(topLeft);
    if(standardItem->hasChildren())
    {
        for(int i = 0; i < standardItem->rowCount(); i ++)
        {
            if(standardItem->child(i)->isEnabled())
                standardItem->child(i)->setCheckState(standardItem->checkState());
        }
    }
    else if(standardItem->parent())
    {
        if(standardItem->checkState() == Qt::Unchecked)
        {
            int isChecked = 0;
            QStandardItem* parentItem = standardItem->parent();
            for(int i = 0; i < parentItem->rowCount(); i ++)
            {
                if(parentItem->child(i)->checkState() != Qt::Unchecked)
                    isChecked = 1;
            }

            if(isChecked == 0)
                parentItem->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::slotSearchBlackResultSelectionChanged(QItemSelection,QItemSelection)
{
    if(!m_selectionModelSearchPageBlackList->hasSelection())
        return;

    QModelIndex index = m_selectionModelSearchPageBlackList->currentIndex();
    int blackIndex = m_modelSearchPageBlackList->data(m_modelSearchPageBlackList->index(index.row(), 0), ROW_INDEX_ROLE).toInt();
    BLACK_RECOG_RESULT candidateResult;
    QByteArray cacheData;
    cacheData.resize(CACHE_BLACK_SIZE);
    FILE* fp = fopen(CACHE_BLACK_NAME, "rb");
    if(!fp)
        return;

    fseek(fp, CACHE_BLACK_SIZE * (m_searchCurBlackPageIndex * N_SEARCH_BLACK_PAGE + blackIndex), SEEK_SET);
    fread(cacheData.data(), cacheData.size(), 1, fp);
    fclose(fp);

    QBuffer cacheBuffer(&cacheData);
    cacheBuffer.open(QIODevice::ReadOnly);
    QDataStream cacheStream(&cacheBuffer);

    cacheStream >> candidateResult.name;
    cacheStream >> candidateResult.gender;
    cacheStream >> candidateResult.birthday;
    cacheStream >> candidateResult.address;
    cacheStream >> candidateResult.description;
    cacheStream >> candidateResult.personType;
    cacheStream >> candidateResult.galleryFaceData;
    cacheStream >> candidateResult.probeFaceData;
    cacheStream >> candidateResult.similiarity;
    cacheStream >> candidateResult.areaName;
    cacheStream >> candidateResult.logUID;
    cacheStream >> candidateResult.dateTime;
    cacheStream >> candidateResult.serverUID;
    cacheStream >> candidateResult.chanelIndex;
    cacheBuffer.close();

    QVector<BLACK_RECOG_RESULT> blackRecogResult;
    blackRecogResult.append(candidateResult);

    ui->widgetSearchBlackResult->setBlackResult(blackRecogResult);
}

void MainWindow::slotSearchBlackResultDoubleClicked(QModelIndex)
{
    if(!m_selectionModelSearchPageBlackList->hasSelection())
        return;

    QModelIndex index = m_selectionModelSearchPageBlackList->currentIndex();

    int blackIndex = m_modelSearchPageBlackList->data(m_modelSearchPageBlackList->index(index.row(), 0), ROW_INDEX_ROLE).toInt();
    BLACK_RECOG_RESULT candidateResult;
    QByteArray cacheData;
    cacheData.resize(CACHE_BLACK_SIZE);
    FILE* fp = fopen(CACHE_BLACK_NAME, "rb");
    if(!fp)
        return;

    fseek(fp, CACHE_BLACK_SIZE * (m_searchCurBlackPageIndex * N_SEARCH_BLACK_PAGE + blackIndex), SEEK_SET);
    fread(cacheData.data(), cacheData.size(), 1, fp);
    fclose(fp);

    QBuffer cacheBuffer(&cacheData);
    cacheBuffer.open(QIODevice::ReadOnly);
    QDataStream cacheStream(&cacheBuffer);

    cacheStream >> candidateResult.name;
    cacheStream >> candidateResult.gender;
    cacheStream >> candidateResult.birthday;
    cacheStream >> candidateResult.address;
    cacheStream >> candidateResult.description;
    cacheStream >> candidateResult.personType;
    cacheStream >> candidateResult.galleryFaceData;
    cacheStream >> candidateResult.probeFaceData;
    cacheStream >> candidateResult.similiarity;
    cacheStream >> candidateResult.areaName;
    cacheStream >> candidateResult.logUID;
    cacheStream >> candidateResult.dateTime;
    cacheStream >> candidateResult.serverUID;
    cacheStream >> candidateResult.chanelIndex;
    cacheBuffer.close();

    QVector<BLACK_RECOG_RESULT> blackRecogResult;
    blackRecogResult.append(candidateResult);

    int exist = -1;
    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        if(m_serverInfoSockets[i]->serverInfo().serverUID == candidateResult.serverUID)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
        return;

    LogSelectDlg dlg;
    dlg.setInfo(m_areaInfos, m_serverInfoSockets, blackRecogResult);
    dlg.exec();

    refreshSelectedBlackInfoByArea();
}

void MainWindow::refreshSearchPageArea()
{
    m_modelSearchPageLogArea->removeRows(0, m_modelSearchPageLogArea->rowCount());
    m_modelSearchPageBlackArea->removeRows(0, m_modelSearchPageBlackArea->rowCount());

    for(int i = 0; i < m_areaInfos.size(); i ++)
    {
        MonitoringAreaInfo monitoringAreaInfo = m_areaInfos[i];
        QStandardItem* logAreaInfoItem = new QStandardItem;
        logAreaInfoItem->setText(monitoringAreaInfo.areaName);
        logAreaInfoItem->setIcon(QIcon(QPixmap(":/images/monitoring_area.png")));

        logAreaInfoItem->setData(i, AREA_INDEX_ROLE);

        QStandardItem* blackAreaInfoItem = logAreaInfoItem->clone();

        for(int j = 0; j < monitoringAreaInfo.serverIndexs.size(); j ++)
        {
            int selectedServerIndex = monitoringAreaInfo.serverIndexs[j];
            int selectedChanelIndex = monitoringAreaInfo.chanelIndexs[j];

            ServerInfo serverInfo = m_serverInfoSockets[selectedServerIndex]->serverInfo();
            int serverStatus = m_serverInfoSockets[selectedServerIndex]->status();
            int chanelStatus = m_serverInfoSockets[selectedServerIndex]->getChanelStatus(selectedChanelIndex);

            QString serverInfoName = serverInfo.serverName;
            QString cameraItemText = serverInfoName + " (" + StringTable::Str_Chanel + QString::number(selectedChanelIndex + 1) + ")";
            QStandardItem* cameraItem = new QStandardItem;
            cameraItem->setText(cameraItemText);
            cameraItem->setCheckable(true);

            if(chanelStatus == CHANEL_STATUS_STOP)
            {
                if(serverStatus == SERVER_STOP)
                    cameraItem->setEnabled(false);

                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera_disable1.png")));
            }
            else
                cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

            cameraItem->setData(i, AREA_INDEX_ROLE);
            cameraItem->setData(selectedServerIndex, SERVER_INDEX_ROLE);
            cameraItem->setData(selectedChanelIndex, CHANEL_INDEX_ROLE);

            logAreaInfoItem->appendRow(cameraItem);
            logAreaInfoItem->setCheckable(true);

            blackAreaInfoItem->appendRow(cameraItem->clone());
            blackAreaInfoItem->setCheckable(true);
        }

        m_modelSearchPageLogArea->appendRow(logAreaInfoItem);
        ui->viewSearchPageLogArea->setExpanded(m_modelSearchPageLogArea->index(i, 0), true);

        m_modelSearchPageBlackArea->appendRow(blackAreaInfoItem);
        ui->viewSearchPageBlackArea->setExpanded(m_modelSearchPageBlackArea->index(i, 0), true);
    }
}

void MainWindow::relocateSearchLogResults()
{
    int columnWidth = CACHE_LOG_ITEM_WIDTH + 20;
    int rowHeight = CACHE_LOG_ITEM_HEIGHT + 20;
    int viewWidth = ui->viewSeachPageLogResult->width() - 5;

    int columnCount = qMax(viewWidth / columnWidth, 1);

    QRect sceneRect(0, 0, viewWidth, ((m_searchPageLogResuttItems.size() + columnCount - 1) / columnCount) * rowHeight);
    m_sceneSearchPageLogResult->setSceneRect(sceneRect);

    for(int i = 0; i < m_searchPageLogResuttItems.size(); i ++)
    {
        int row = i / columnCount;
        int col = i % columnCount;

        m_searchPageLogResuttItems[i]->setPos(10 + col* columnWidth, 10 + row * rowHeight);
    }

    m_sceneSearchPageLogResult->update();
}

void MainWindow::refreshSearchLogFace()
{
#define SEARCH_FACE_WIDTH 96
#define SEARCH_FACE_HEIGHT 120

    if(m_searchLogFaceInfo.faceImage.isNull())
    {
        QPixmap emptyPixmap(SEARCH_FACE_WIDTH, SEARCH_FACE_HEIGHT);
        QPainter painter;
        painter.begin(&emptyPixmap);
        painter.fillRect(emptyPixmap.rect(), Qt::white);

        painter.setPen(Qt::black);
        painter.drawRect(QRect(0, 0, SEARCH_FACE_WIDTH - 1, SEARCH_FACE_HEIGHT - 1));
        painter.end();

        ui->lblSearchLogFace->setPixmap(emptyPixmap);
    }
    else
    {
        QImage scaledImage = m_searchLogFaceInfo.faceImage.scaled(QSize(SEARCH_FACE_WIDTH, SEARCH_FACE_HEIGHT), Qt::KeepAspectRatio);
        ui->lblSearchLogFace->setPixmap(QPixmap::fromImage(scaledImage));
    }
}



void MainWindow::stopSearchLogSockets()
{
    if(m_searchLogSocket)
    {
        m_searchLogSocket->stopSocket();
        delete m_searchLogSocket;
        m_searchLogSocket = NULL;
    }

    ui->btnSearchLog->setText(StringTable::Str_Search);
    ui->progressSearchLog->hide();
}


void MainWindow::stopSearchBlackSockets()
{
    if(m_searchBlackSocket)
    {
        m_searchBlackSocket->stopSocket();
        delete m_searchBlackSocket;
        m_searchBlackSocket = NULL;
    }

    ui->btnSearchBlack->setText(StringTable::Str_Search);
    ui->progressSearchBlack->hide();
}


void MainWindow::stopSearchLastBlackSockets()
{
    if(m_searchLastBlackSocket)
    {
        m_searchLastBlackSocket->stopSocket();
        delete m_searchLastBlackSocket;
        m_searchLastBlackSocket = NULL;
    }
}

void MainWindow::retranslateUI()
{
    ui->actionMonitoringPage->setText(StringTable::Str_Monitoring);
    ui->actionServer->setText(StringTable::Str_Machines);
    ui->actionSetting->setText(StringTable::Str_Setting);
    ui->actionAddServer->setText(StringTable::Str_Add);
    ui->actionEditServer->setText(StringTable::Str_Edit);
    ui->actionDeleteServer->setText(StringTable::Str_Delete);
    ui->actionEditIPCameras->setText(StringTable::Str_Chanel_Setting);
    ui->actionMonitoringArea->setText(StringTable::Str_Monitoring_Area);
    ui->actionAddArea->setText(StringTable::Str_Add);
    ui->actionEditArea->setText(StringTable::Str_Edit);
    ui->actionDeleteArea->setText(StringTable::Str_Delete);
    ui->actionBlackList->setText(StringTable::Str_Black_DB);
    ui->actionEditBlack->setText(StringTable::Str_Edit);
    ui->actionDeleteBlack->setText(StringTable::Str_Delete);
    ui->actionAddBlackFromFile->setText(StringTable::Str_Add_from_file);
    ui->actionAddBlackFromBatch->setText(StringTable::Str_Add_from_batch);
    ui->actionSurveillanceSetting->setText(StringTable::Str_Surveillance_Setting);
    ui->actionCameraSetting->setText(StringTable::Str_Camera_Setting);
    ui->actionSearchPage->setText(StringTable::Str_Search);
    ui->actionSearchPageLog->setText(StringTable::Str_Search_Log);
    ui->actionSearchPageBlack->setText(StringTable::Str_Search_Black);

    ui->groupDateTime->setTitle(StringTable::Str_Date_Time);
    ui->groupFace->setTitle(StringTable::Str_Face);
    ui->lblLogSearchStart->setText(StringTable::Str_Start_Time);
    ui->lblLogSearchEnd->setText(StringTable::Str_End_Time);
    ui->lblThresholdLog->setText(StringTable::Str_Threshold);
    ui->btnSearchLog->setText(StringTable::Str_Search);

    ui->groupDateTimeBlack->setTitle(StringTable::Str_Date_Time);
    ui->lblLogSearchStartBlack->setText(StringTable::Str_Start_Time);
    ui->lblLogSearchEndBlack->setText(StringTable::Str_End_Time);
    ui->groupNameBlack->setTitle(StringTable::Str_Name);
    ui->btnSearchBlack->setText(StringTable::Str_Search);
    ui->btnSetFace->setText(StringTable::Str_Set_Face);
    ui->actionAbout->setText(StringTable::Str_About);

    ui->actionExportBlack->setText(StringTable::Str_Export);
    ui->actionImportBlack->setText(StringTable::Str_Import);
    ui->actionSendToBlack->setText(StringTable::Str_Send_to);

    m_menuPassword->menuAction()->setText(StringTable::Str_Password);
    ui->actionNewPassword->setText(StringTable::Str_New_assword);
    ui->actionChangePassword->setText(StringTable::Str_Change_Password);
    ui->actionRemovePassword->setText(StringTable::Str_Remove_Password);

    m_resultAlarmDlg->retranslateUI();

    setWindowTitle(StringTable::Str_VMSClient + tr(" - Video"));
}

void MainWindow::slotExportBlack()
{
    if(m_selectedBlackMetaInfo.size() == 0)
        return;

    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString selectedFileName = QFileDialog::getSaveFileName(this, StringTable::Str_Save_File, setting.value("export black dir").toString() + "black info video.db", "Black Info(*.db)");
    if(selectedFileName.isEmpty())
        return;

    QFileInfo info(selectedFileName);
    setting.setValue("export black dir", info.absoluteDir().absolutePath());

    QFile blackFile(selectedFileName);
    if(!blackFile.open(QIODevice::WriteOnly))
        return;

    QDataStream blackStream(&blackFile);

    blackStream << m_selectedBlackMetaInfo.size();

    for(int i = 0; i < m_selectedBlackMetaInfo.size(); i ++)
    {
        QCryptographicHash crphytoHash(QCryptographicHash::Sha224);
        for(int j = 0; j < m_selectedBlackMetaInfo[i].featData.size(); j ++)
            crphytoHash.addData(m_selectedBlackMetaInfo[i].featData[j]);

        blackStream << m_selectedBlackMetaInfo[i].featData;
        blackStream << m_selectedBlackMetaInfo[i].faceData;
        blackStream << m_selectedBlackMetaInfo[i].name;
        blackStream << m_selectedBlackMetaInfo[i].gender;
        blackStream << m_selectedBlackMetaInfo[i].birthDay;
        blackStream << m_selectedBlackMetaInfo[i].address;
        blackStream << m_selectedBlackMetaInfo[i].description;
        blackStream << m_selectedBlackMetaInfo[i].personType;

        QByteArray checkSumData = crphytoHash.result();
        blackStream << checkSumData;
    }

    blackFile.close();
}

void MainWindow::slotImportBlack()
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString selectedFileName = QFileDialog::getOpenFileName(this, StringTable::Str_Open_File, setting.value("export black dir").toString(),
                                                    tr("Black Info(*.db)"));
    if(selectedFileName.isEmpty())
        return;

    QFileInfo info(selectedFileName);
    setting.setValue("export black dir", info.absoluteDir().absolutePath());

    QFile blackFile(selectedFileName);
    if(!blackFile.open(QIODevice::ReadOnly))
        return;

    QDataStream blackStream(&blackFile);

    QVector<BLACK_PERSON> loadBlackData;

    int blackInfoSize = 0;
    blackStream >> blackInfoSize;

    for(int i = 0; i < blackInfoSize; i ++)
    {
        BLACK_PERSON blackPerson;
        blackStream >> blackPerson.featData;
        blackStream >> blackPerson.faceData;
        blackStream >> blackPerson.name;
        blackStream >> blackPerson.gender;
        blackStream >> blackPerson.birthDay;
        blackStream >> blackPerson.address;
        blackStream >> blackPerson.description;
        blackStream >> blackPerson.personType;

        loadBlackData.append(blackPerson);

        QByteArray checkSumData;
        blackStream >> checkSumData;

        QCryptographicHash crphytoHash(QCryptographicHash::Sha224);
        for(int j = 0; j < blackPerson.featData.size(); j ++)
            crphytoHash.addData(blackPerson.featData[j]);

        if(memcmp(crphytoHash.result().data(), checkSumData.data(), qMin(checkSumData.size(), crphytoHash.result().size())))
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Unkown_format);
            blackFile.close();
            return;
        }
    }

    blackFile.close();

    if(loadBlackData.size() == 0)
        return;

    m_progressDialog->close();
    stopAddBlackInfoSocket();

    m_addBlackInfoSocket = new AddBlackInfoSocket;
    m_addBlackInfoSocket->setInfo(m_serverInfoSockets[m_selectedServerIndexOfEditBlack]->serverInfo(), m_selectedChanelIndexOfEditBlack, loadBlackData);

    connect(m_addBlackInfoSocket, SIGNAL(enrollFinished(QObject*)), this, SLOT(slotAddBlackInfoSocketFinished(QObject*)));

    m_addBlackInfoSocket->startSocket();
    refreshActions();
}

void MainWindow::slotSendToBlack()
{
    if(m_selectedBlackMetaInfo.size() == 0)
        return;

    SelectChanelDlg dlg;

    dlg.setInfo(m_areaInfos, m_serverInfoSockets);
    int ret = dlg.exec();
    if(!ret)
        return;

    QVector<int> serverIndexs, chanelIndexs;

    dlg.getSelectedInfo(serverIndexs, chanelIndexs);

    if(serverIndexs.size() == 0)
        return;

    stopSendToBlack();
    for(int i = 0; i < serverIndexs.size(); i ++)
    {
        if(serverIndexs[i] != m_selectedServerIndexOfEditBlack || chanelIndexs[i] != m_selectedChanelIndexOfEditBlack)
        {
            AddBlackInfoSocket* addBlackInfoSocket = new AddBlackInfoSocket;
            m_sendBlackInfoSocekts.append(addBlackInfoSocket);

            addBlackInfoSocket->setInfo(m_serverInfoSockets[serverIndexs[i]]->serverInfo(), chanelIndexs[i], m_selectedBlackMetaInfo);
            connect(addBlackInfoSocket, SIGNAL(enrollFinished(QObject*)), this, SLOT(sendToBlackFinished(QObject*)));

            addBlackInfoSocket->startSocket();
        }
    }

    ui->progressBarBlack->show();
//    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void MainWindow::sendToBlackFinished(QObject* obj)
{
    for(int i = 0; i < m_sendBlackInfoSocekts.size(); i ++)
    {
        if(m_sendBlackInfoSocekts[i] == obj)
        {
            m_sendBlackInfoSocekts[i]->stopSocket();
            delete m_sendBlackInfoSocekts[i];
            m_sendBlackInfoSocekts.remove(i);
        }
    }

    if(m_sendBlackInfoSocekts.size() == 0)
    {
        ui->progressBarBlack->hide();
//        QApplication::restoreOverrideCursor();
    }
}

void MainWindow::stopSendToBlack()
{
    for(int i = 0; i < m_sendBlackInfoSocekts.size(); i ++)
    {
        m_sendBlackInfoSocekts[i]->stopSocket();
        delete m_sendBlackInfoSocekts[i];
    }

    m_sendBlackInfoSocekts.clear();
}

void MainWindow::slotNewPassword()
{
    QString passStr = readPass();
    if(!passStr.isEmpty())
        return;

    NewPass newDlg;
    newDlg.exec();

    refreshActions();
}

void MainWindow::slotChangePassword()
{
    QString passStr = readPass();
    if(passStr.isEmpty())
        return;

    ModifyPass modifyDlg;
    modifyDlg.exec();

    refreshActions();
}

void MainWindow::slotRemovePassword()
{
    QString passStr = readPass();
    if(passStr.isEmpty())
        return;

    RemovePass removeDlg;
    removeDlg.exec();

    refreshActions();
}

void MainWindow::showResultAlarmDlg()
{
#ifdef _SHOW_ALAM_
    for(int i = m_monitoringPageBlackResultModel->rowCount() - 1; i >= 0; i --)
    {
        QStandardItem* groupItem = m_monitoringPageBlackResultModel->item(i);
        if(groupItem->data(ALRM_FLAG_ROLE).toInt() == 1 && m_resultAlarmDlg->alarmID() == -1)
        {
            int alarmId = groupItem->data(ALRM_ID_ROLE).toInt();

            groupItem->setData(0, ALRM_FLAG_ROLE);

            int selectedIndex = -1;
            QString logUID = groupItem->data(LOG_UID_ROLE).toString();
            for(int j = 0; j < m_blackRecogResults.size(); j ++)
            {
                if(m_blackRecogResults[j][0].logUID == logUID)
                {
                    selectedIndex = j;
                    break;
                }
            }

            if(selectedIndex < 0)
                return;

            m_resultAlarmDlg->setData(alarmId, m_blackRecogResults[selectedIndex]);
            break;
        }
    }
#endif
}


