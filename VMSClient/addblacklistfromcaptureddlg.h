#ifndef ADDBLACKLISTFROMCAPTUREDDLG_H
#define ADDBLACKLISTFROMCAPTUREDDLG_H

#include "clientbase.h"
#include <QDialog>

namespace Ui {
class AddBlackListFromCapturedDlg;
}

class ServerInfoSocket;
class ImageProcessingSocket;
class FrameResultItem;
class NextButtonItem;
class AddBlackInfoSocket;
class AddBlackListFromCapturedDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AddBlackListFromCapturedDlg(QWidget *parent = 0);
    ~AddBlackListFromCapturedDlg();

    void    setInfo(QVector<MonitoringAreaInfo> monitoringAreaInfos, QVector<ServerInfoSocket*> serverInfoSockets, int selectedAreaIndex, int selectedChanelIndex, QImage sceneImage, int processEngine);
    void    addFrameResult(FRAME_RESULT frameResult);

private slots:
    void    slotAreaItemDataChanged(QModelIndex topLeft, QModelIndex topRight, QVector<int> roles);
    void    slotReceiveFrameResults(QVector<FRAME_RESULT>);

    void    slotDownClicked();
    void    slotUpClicked();

    void    slotDetectionPrevClicked();
    void    slotDetectionNextClicked();
    void    slotEnrollPrevClicked();
    void    slotEnrollNextClicked();
    void    slotOk();
    void    slotCancel();
    void    slotAddOneBlackPersonFinishded(QObject* obj);

    void    startImageProcessing();
    void    stopImageProcessing();


private:
    void    setupActions();
    void    refreshAreaInfo();
    void    updateSceneImage();

    void    constructDetectionResultItems();
    void    refreshDetectionResultItems();
    void    relocateDetectionResultItems();

    void    constructEnrollResultItems();
    void    refreshEnrollResultItems();
    void    relocateEnrollResultItems();
    void    retranslateUI();
protected:
    void    resizeEvent(QResizeEvent* e);
    void    paintEvent(QPaintEvent *);

private:
    Ui::AddBlackListFromCapturedDlg *ui;

    QVector<MonitoringAreaInfo>     m_monitoringAreaInfos;
    QVector<ServerInfoSocket*>      m_serverInfoSockets;
    QImage                          m_sceneImage;

    QGraphicsScene*                 m_detectionScene;
    NextButtonItem*                 m_detectionPrevItem;
    NextButtonItem*                 m_detectionNextItem;
    QVector<FrameResultItem*>       m_detectionFrameItems;
    QVector<FRAME_RESULT>           m_detectionFrameResults;

    QGraphicsScene*                 m_enrollScene;
    NextButtonItem*                 m_enrollPrevItem;
    NextButtonItem*                 m_enrollNextItem;
    QVector<FrameResultItem*>       m_enrollFrameItems;
    QVector<FRAME_RESULT>           m_enrollFrameResults;

    QStandardItemModel*     m_monitoringAreaModel;
    QItemSelectionModel*    m_monitoringAreaSelectionModel;

    ImageProcessingSocket*  m_imageProessingSocket;
    QVector<AddBlackInfoSocket*>     m_addOneBlackPersonSockets;

    int                     m_selectedAreaIndex;
    int                     m_selectedServerIndex;
    int                     m_selectedChanelIndex;
};

#endif // ADDBLACKLISTFROMCAPTUREDDLG_H
