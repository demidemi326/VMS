#ifndef EDITBLACKINFODLG_H
#define EDITBLACKINFODLG_H

#include "clientbase.h"

#include <QDialog>

namespace Ui {
class EditBlackInfoDlg;
}

class NextButtonItem;
class FrameResultItem;
class ServerInfoSocket;
class EditOneBlackListSocket;
class EditBlackInfoDlg : public QDialog
{
    Q_OBJECT

public:
    explicit EditBlackInfoDlg(QWidget *parent = 0);
    ~EditBlackInfoDlg();

    void    setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex, BLACK_PERSON blackPerson);
    BLACK_PERSON    blackPersonInfo();

public slots:
    void    slotAddFace();
    void    slotDeleteFace();
    void    slotOk();
    void    slotCancel();
    void    slotEnrollPrevClicked();
    void    slotEnrollNextClicked();

protected:
    void    resizeEvent(QResizeEvent* e);


private:
    void    setupActions();

    void    constructEnrollResultItems();
    void    refreshEnrollResultItems();
    void    relocateEnrollResultItems();
    void    retranslateUI();

private:
    Ui::EditBlackInfoDlg *ui;

    QGraphicsScene*                 m_enrollScene;
    NextButtonItem*                 m_enrollPrevItem;
    NextButtonItem*                 m_enrollNextItem;
    QVector<FrameResultItem*>       m_enrollFrameItems;
    QVector<FRAME_RESULT>           m_enrollFrameResults;

    ServerInfoSocket*               m_serverInfoSocket;
    int                             m_chanelIndex;

    BLACK_PERSON                    m_blackPersonInfo;
};

#endif // EDITBLACKINFODLG_H
