#ifndef ADDBLACKFACEDLG_H
#define ADDBLACKFACEDLG_H

#include <QDialog>

#include "clientbase.h"

namespace Ui {
class BlackFaceAddDlg;
}

class QGraphicsScene;
class NextButtonItem;
class FrameResultItem;
class ImageProcessingSocket;
class ServerInfoSocket;
class BlackFaceAddDlg : public QDialog
{
    Q_OBJECT

public:
    explicit BlackFaceAddDlg(QWidget *parent = 0);
    ~BlackFaceAddDlg();
    void    setInfo(ServerInfoSocket* serverInfoSocket, int maximumCount);

    QVector<FRAME_RESULT> enrollFrameResults();

public slots:
    void    slotOk();
    void    slotCancel();
    void    slotDown();
    void    slotUp();
    void    slotAddImage();

    void    slotDetectionPrevClicked();
    void    slotDetectionNextClicked();
    void    slotEnrollPrevClicked();
    void    slotEnrollNextClicked();

private:
    void    constructDetectionResultItems();
    void    refreshDetectionResultItems();
    void    relocateDetectionResultItems();
    void    constructEnrollResultItems();
    void    refreshEnrollResultItems();
    void    relocateEnrollResultItems();

    void    setSelectedImage(QImage);
    void    updateSceneImage();

private slots:
    void    startImageProcessing();
    void    stopImageProcessing();
    void    receiveFrameResults(QVector<FRAME_RESULT>);
    void    retranslateUI();

protected:
    void    resizeEvent(QResizeEvent* e);

private:
    Ui::BlackFaceAddDlg *ui;

    QToolBar*                   m_toolbar;

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

    QImage                          m_sceneImage;

    ImageProcessingSocket*          m_imageProessingSocket;
    ServerInfoSocket*               m_serverInfoSocket;
    int                             m_maximumCount;
};

#endif // ADDBLACKFACEDLG_H
