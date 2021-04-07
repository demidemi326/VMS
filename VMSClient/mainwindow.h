#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "clientbase.h"

namespace Ui {
class MainWindow;
}

class QMenu;
class QToolBar;
class QGraphicsScene;
class QStandardItemModel;
class QItemSelectionModel;
class MediaItem;
class MediaItem_GL;
class LogResultReceiveSocket;
class ServerInfoSocket;
class LogViewItem;
class BlackInfoReceiveSocket;
class DeleteBlackListSocket;
class BlackRecogResultReceiveSocket;
class EditOneBlackListSocket;
class AddBlackInfoSocket;
class SearchLogSocket;
class SearchBlackSocket;
class CacheLogViewItem;
class SearchLastBlackSocket;
class ResultAlarmDlg;
class CameraSurfaceGLItem;
class QGraphicsProxyWidget;
class CameraSurfaceView;
class QProgressDialog;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void    slotViewMonitoringPage();
    void    slotViewServerPage();
    void    slotViewAreaPage();
    void    slotViewBlackListPage();
    void    slotViewSearchPage();
    void    slotAbout();

    void    slotViewCamera1_1();
    void    slotViewCamera2_2();
    void    slotViewCamera3_3();
    void    slotViewCamera4_4();    
    void    slotCapture();

    void    slotMonitoringPageAreaItemDoubleClicked(QModelIndex);
    void    slotMonitoringPageAreaItemContext(QPoint pos);
    void    slotMonitoringPageCameraSetting();
    void    slotMonitoringPageSurveillanceSetting();

    void    slotBlackResultSelectionChanged(QItemSelection,QItemSelection);
    void    slotBlackResultDoubleClicked(QModelIndex);

    void    slotMediaSelectionChanged(CameraSurfaceView* selectedView);
    void    slotMediaMaximumChanged();
    void    slotReceiveLogResult(LOG_RESULT);
    void    slotReceiveBlackRecogResults(QVector<BLACK_RECOG_RESULT> blackRecogResult);
    void    slotReceiveLastBlackResult(QVector<BLACK_RECOG_RESULT>);
    void    slotLogItemDoubleClicked(LOG_RESULT);

    void    slotAddServer();
    void    slotEditServer();
    void    slotDeleteServer();
    void    slotEditIpCamera();

    void    slotServerPageServerItemDoubleClicked(QModelIndex);
    void    slotServerPageServerItemSelectionChanged(QItemSelection,QItemSelection);
    void    slotServerStatusChanged(int serverIndex, int status);
    void    slotServerChanelStatusChanged(int serverIndex, int chanelIndex, int status);
    void    slotBlackInfoChanged(int, int);

    void    slotAddArea();
    void    slotEditArea();
    void    slotDeleteArea();

    void    slotAddBlackFromSingleFile();
    void    slotAddBlackFromBatchFile();
    void    slotEditBlackPerson();
    void    slotDeleteBlackPerson();

    void    slotAddCameraToArea();
    void    slotRemoveCameraFromArea();

    void    slotAreaEditItemDoubleClicked(QModelIndex);
    void    slotAreaEditItemDataChanged(QModelIndex topLeft, QModelIndex topRight, QVector<int> roles);

    void    slotAreaServerItemDoubleClicked(QModelIndex);
    void    slotAreaServerItemDataChanged(QModelIndex topLeft, QModelIndex topRight, QVector<int> roles);
    void    slotAreaPageAreaItemSelectionChaned(QItemSelection, QItemSelection);

    void    slotEditBlackAreaSelectionChanged(QItemSelection,QItemSelection);
    void    slotReceivedSelectedBlackInfos(QVector<BLACK_PERSON> blackInfos);
    void    slotBlackInfoItemDoubleClicked(QModelIndex);
    void    slotBlackInfoItemSelectionChanged(QItemSelection, QItemSelection);

    void    slotDeleteBlackInfoSocketFinished(QObject*);
    void    slotAddBlackInfoSocketFinished(QObject*);
    void    slotEditBlackInfoSocketFinished(QObject*);

    void    slotViewSearchLog();
    void    slotViewSearchBlack();

    void    slotSearchLog();
    void    slotSearchBlack();
    void    slotSearchLogSocketFinished(QObject* obj);
    void    slotSearchBlackSocketFinished(QObject* obj);
    void    slotSearchPageLogAreaItemDataChanged(QModelIndex topLeft, QModelIndex topRight, QVector<int> roles);
    void    slotSearchPageBlackAreaItemDataChanged(QModelIndex topLeft, QModelIndex , QVector<int> roles);
    void    slotReceiveSearchLogResult(LOG_RESULT logResult);
    void    slotSearchBlackRecogResults(QVector<BLACK_RECOG_RESULT> blackRecogResult);
    void    slotSearchBlackResultSelectionChanged(QItemSelection,QItemSelection);
    void    slotSearchBlackResultDoubleClicked(QModelIndex);
    void    slotSetFace();
    void    slotThresoldChanged(int threshold);

    void    slotFirstLog();
    void    slotBackwardLog();
    void    slotPrevLog();
    void    slotNextLog();
    void    slotForwardLog();
    void    slotLastLog();

    void    slotFirstBlack();
    void    slotBackwardBlack();
    void    slotPrevBlack();
    void    slotNextBlack();
    void    slotForwardBlack();
    void    slotLastBlack();

    void    slotExportBlack();
    void    slotImportBlack();
    void    slotSendToBlack();

    void    slotNewPassword();
    void    slotChangePassword();
    void    slotRemovePassword();

protected:
    void    resizeEvent(QResizeEvent* e);
    bool    eventFilter(QObject *, QEvent *);
    void    timerEvent(QTimerEvent* e);

private slots:
    void    setupLogViewItems();
    void    refreshActions();

private:
    void    loadSetting();
    void    saveSetting();
    void    loadLog();
    void    saveLog();

    void    loadViewSetting();
    void    saveViewSetting();

    void    setupActions();
    void    setupSockets();

    void    setupCameraViews();
    void    relocateCameraViewItems();
    void    initCameraViewItem();
    void    refreshCameraBar();

    void    relocateLogViewItems();
    void    refreshMonitoringPageArea();
    void    refreshMonitoringPageBlackResult();

    void    refreshServers();
    void    addServerSocket(ServerInfo serverInfo);
    void    addLogFaceReceiveSocket(int serverIndex, int chanelIndex);
    void    addBlackRecogResultReceiveSocket(int serverIndex, int chanelIndex);

    void    refreshAreaPageArea();
    int     selectedEditAreaIndex();

    void    editSelectedIpCamera(int serverIndex, int chanelIndex);

    void    refreshBlackPageArea();
    void    refreshSelectedBlackMetaInfos();
    void    refreshSelectedBlackInfoByArea();
    void    stopBlackInfoReceive();
    void    stopBlackInfoDelete();
    void    stopAddBlackInfoSocket();
    void    stopEditBlackInfoSocket();

    void    refreshSearchPageArea();
    void    relocateSearchLogResults();

    void    refreshSearchLogFace();
    void    stopSearchBlackSockets();
    void    stopSearchLogSockets();

    void    retranslateUI();

    void    refreshLogNavigation();
    void    refreshCurLog();

    void    refreshBlackNavigation();
    void    refreshCurBlack();

    void    stopSearchLastBlackSockets();

private slots:
    void    sendToBlackFinished(QObject* obj);
    void    stopSendToBlack();

    void    showResultAlarmDlg();


private:
    Ui::MainWindow *ui;

    QVector<MonitoringAreaInfo>         m_areaInfos;

    QVector<ServerInfoSocket*>          m_serverInfoSockets;
    QVector<LogResultReceiveSocket*>      m_logImageReceiveSockets;
    QVector<BlackRecogResultReceiveSocket*> m_blackRecogResultReceiveSockets;
    BlackInfoReceiveSocket*             m_blackInfoReceiveSocket;
    QVector<BLACK_PERSON>               m_selectedBlackMetaInfo;

    AddBlackInfoSocket*                 m_addBlackInfoSocket;
    EditOneBlackListSocket*             m_editBlackInfoSocket;

    int             m_selectedServerIndexOfEditBlack;
    int             m_selectedChanelIndexOfEditBlack;

    QGraphicsScene*         m_sceneMonitoringPageCamera;
    //QVector<MediaItem*>     m_cameraViewItems;
    QVector<CameraSurfaceView*>   m_cameraViewItems;
    int                     m_cameraViewType;


    QGraphicsScene*         m_sceneMonitoringPageLog;
    QVector<LOG_RESULT>     m_faceImageInfos;
    QVector<LogViewItem*>   m_logViewItem;

    QVector<QVector<BLACK_RECOG_RESULT> > m_blackRecogResults;

    QToolBar*               m_toolbarMonitoringPageCamera;
    QToolBar*               m_toolbarMonitoringPageLog;

    QStandardItemModel*     m_monitoringPageAreaModel;
    QItemSelectionModel*    m_monitoringPageAreaSelectionModel;
    QMenu*                  m_monitoringAreaContextMenu;

    QStandardItemModel*     m_monitoringPageBlackResultModel;
    QItemSelectionModel*    m_monitoringPageBlackResultSelectionModel;


    QToolBar*               m_toolbarServerPageServer;
    QStandardItemModel*     m_modelServerPageServer;
    QItemSelectionModel*    m_selectionModelServerPageServer;

    QToolBar*               m_toolbarAreaPageArea;
    QStandardItemModel*     m_modelAreaPageArea;
    QItemSelectionModel*    m_selectionModelAreaPageArea;
    QStandardItemModel*     m_modelAreaPageServer;
    QItemSelectionModel*    m_selectionModelAreaPageServer;
    int                     m_areaEditFlag;

    QStandardItemModel*     m_modelBlackPageArea;
    QItemSelectionModel*    m_selectionModelBlackPageArea;

    QToolBar*               m_toolbarBlackPage;
    QStandardItemModel*     m_modelBlackPageBlack;
    QItemSelectionModel*    m_selectionModelBlackPageBlack;

    DeleteBlackListSocket*  m_deletBlackListSocket;

    QToolBar*               m_toolbarSearchPage;
    QStandardItemModel*     m_modelSearchPageLogArea;
    QItemSelectionModel*    m_selectionModelSearchPageLogArea;

    QStandardItemModel*     m_modelSearchPageBlackArea;
    QItemSelectionModel*    m_selectionModelSearchPageBlackArea;

    QGraphicsScene*         m_sceneSearchPageLogResult;
    QVector<CacheLogViewItem*>   m_searchPageLogResuttItems;

    QStandardItemModel*     m_modelSearchPageBlackList;
    QItemSelectionModel*    m_selectionModelSearchPageBlackList;


    FRAME_RESULT            m_searchLogFaceInfo;

    SearchLogSocket*        m_searchLogSocket;
    int                     m_searchLogCount;
    int                     m_searchCurLogPageIndex;
    int                     m_searchAllLogPageCount;

    SearchBlackSocket*      m_searchBlackSocket;
    int                     m_searchBlackCount;
    int                     m_searchCurBlackPageIndex;
    int                     m_searchAllBlackPageCount;

    int     m_timerID;

    SearchLastBlackSocket*  m_searchLastBlackSocket;

    int     m_firstStart;

    QVector<AddBlackInfoSocket*>    m_sendBlackInfoSocekts;
    QMenu*      m_menuPassword;

    ResultAlarmDlg*         m_resultAlarmDlg;

    int         m_blackAlarmId;
    QProgressDialog*        m_progressDialog;
};

#endif // MAINWINDOW_H
