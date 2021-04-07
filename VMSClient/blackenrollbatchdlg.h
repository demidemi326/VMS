#ifndef BLACKENROLLBATCHDLG_H
#define BLACKENROLLBATCHDLG_H

#include "clientbase.h"
#include <QDialog>


namespace Ui {
class BlackEnrollBatchDlg;
}

class QToolBar;
class QStandardItemModel;
class QItemSelectionModel;
class ImageSearchEngine;
class ImageProcessingSocket;
class ServerInfoSocket;
class QProgressDialog;
class BlackEnrollBatchDlg : public QDialog
{
    Q_OBJECT

public:
    explicit BlackEnrollBatchDlg(QWidget *parent = 0);
    ~BlackEnrollBatchDlg();

    void    setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex);
    QVector<BLACK_PERSON>   blackPersonInfos();

public slots:
    void    slotSelectFiles();
    void    slotSelectFolders();
    void    slotEditBlackBatch();
    void    slotDeleteBlackBatch();
    void    slotProcessBlackBatch();
    void    slotOk();
    void    slotCancel();
    void    slotProgressCanceled();

    void    slotPersonTypeChanged(int personType);
private slots:
    void    addImageFiles(QStringList);
    void    processCurrentImage();
    void    slotReceivedFrameResults(QVector<FRAME_RESULT>);

    void    blackListItemDoubleClicked(QModelIndex);
    void    blackListItemSelectionChanged(QItemSelection, QItemSelection);

private:
    void    refreshBlackBatchList();
    void    retranslateUI();
    void    refreshActions();

protected:
    void    resizeEvent(QResizeEvent* e);

private:
    Ui::BlackEnrollBatchDlg *ui;

    QToolBar*                   m_toolbar;

    QStandardItemModel*     m_batchModel;
    QItemSelectionModel*    m_batchSelectionModel;

    QVector<BLACK_PERSON>               m_batchBlackList;
    QStringList                         m_batchFiles;

    int                                 m_processingIndex;

    ServerInfoSocket*                   m_serverInfoSocket;
    int                                 m_chanelIndex;
    ImageProcessingSocket*              m_imageProcessingSocket;

    ImageSearchEngine*      m_imageSearchEngine;

    QComboBox*      m_comboPersonType;
    QLabel*         m_lblPersonType;
    QProgressDialog* m_progressDialog;
};

#endif // BLACKENROLLBATCHDLG_H
