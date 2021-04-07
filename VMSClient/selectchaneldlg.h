#ifndef SELECTCHANELDLG_H
#define SELECTCHANELDLG_H

#include <QDialog>

#include "clientbase.h"

namespace Ui {
class SelectChanelDlg;
}

class SelectChanelDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SelectChanelDlg(QWidget *parent = 0);
    ~SelectChanelDlg();

    void    setInfo(QVector<MonitoringAreaInfo> areaInfos, QVector<ServerInfoSocket*> serverInfoSockets);
    void    getSelectedInfo(QVector<int>& serverIndexs, QVector<int>& chanelIndexs);

public slots:
    void    slotOk();
    void    slotCancel();

private:
    void    setupActions();
    void    refreshAreas();
    void    retranslateUI();

private slots:
    void    areaItemDataChanged(QModelIndex topLeft, QModelIndex , QVector<int>);

private:
    Ui::SelectChanelDlg *ui;

    QVector<MonitoringAreaInfo> m_areaInfos;
    QVector<ServerInfoSocket*>  m_serverInfoSockets;

    QStandardItemModel*     m_modelArea;
    QItemSelectionModel*    m_selectionModelArea;
};

#endif // SELECTCHANELDLG_H
