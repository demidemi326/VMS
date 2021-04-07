#include "selectchaneldlg.h"
#include "ui_selectchaneldlg.h"
#include "clientbase.h"
#include "stringtable.h"
#include "serverinfosocket.h"

#include <QtWidgets>

SelectChanelDlg::SelectChanelDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectChanelDlg)
{
    ui->setupUi(this);

    setupActions();

    retranslateUI();
}

SelectChanelDlg::~SelectChanelDlg()
{
    delete ui;
}

void SelectChanelDlg::setInfo(QVector<MonitoringAreaInfo> areaInfos, QVector<ServerInfoSocket*> serverInfoSockets)
{
    m_areaInfos = areaInfos;
    m_serverInfoSockets = serverInfoSockets;

    refreshAreas();
}

void SelectChanelDlg::getSelectedInfo(QVector<int>& serverIndexs, QVector<int>& chanelIndexs)
{
    for(int i = 0; i < m_modelArea->rowCount(); i ++)
    {
        QStandardItem* areaItem = m_modelArea->item(i);
        for(int j = 0; j < areaItem->rowCount(); j ++)
        {
            QStandardItem* chanelItem = areaItem->child(j);
            if(chanelItem->isEnabled() && chanelItem->checkState() == Qt::Checked)
            {
                int serverIndex = chanelItem->data(SERVER_INDEX_ROLE).toInt();
                int chanelIndex = chanelItem->data(CHANEL_INDEX_ROLE).toInt();

                serverIndexs.append(serverIndex - 1);
                chanelIndexs.append(chanelIndex - 1);
            }
        }
    }
}

void SelectChanelDlg::slotOk()
{
    int existChecked = 0;
    int existEnabled = 0;
    for(int i = 0; i < m_modelArea->rowCount(); i ++)
    {
        QStandardItem* areaItem = m_modelArea->item(i);
        for(int j = 0; j < areaItem->rowCount(); j ++)
        {
            QStandardItem* chanelItem = areaItem->child(j);
            if(chanelItem->isEnabled())
                existEnabled = 1;

            if(chanelItem->checkState() == Qt::Checked)
                existChecked = 1;
        }
    }

    if(existChecked == 0 && existEnabled == 1)
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_select_chanel);
        return;
    }

    done(1);
}
void SelectChanelDlg::slotCancel()
{
    done(0);
}

void SelectChanelDlg::setupActions()
{
    m_modelArea = new QStandardItemModel(0, 1);
    m_modelArea->setHeaderData(0, Qt::Horizontal, StringTable::Str_Monitoring_Area);

    m_selectionModelArea = new QItemSelectionModel(m_modelArea);

    ui->viewArea->setModel(m_modelArea);
    ui->viewArea->setSelectionModel(m_selectionModelArea);
    ui->viewArea->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->viewArea->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->viewArea->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewArea->setAlternatingRowColors(true);

    connect(m_modelArea, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(areaItemDataChanged(QModelIndex, QModelIndex, QVector<int>)));

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
}


void SelectChanelDlg::refreshAreas()
{
    m_modelArea->removeRows(0, m_modelArea->rowCount());

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
            int serverStatus = m_serverInfoSockets[selectedServerIndex]->status();

            QString serverInfoName = serverInfo.serverName;
            QString cameraItemText = serverInfoName + " (" + StringTable::Str_Chanel + QString::number(selectedChanelIndex + 1) + ")";
            QStandardItem* cameraItem = new QStandardItem;
            cameraItem->setText(cameraItemText);
            cameraItem->setCheckable(true);

            cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));
            if(serverStatus == SERVER_STOP)
                cameraItem->setEnabled(false);

            cameraItem->setData(i, AREA_INDEX_ROLE);
            cameraItem->setData(selectedServerIndex + 1, SERVER_INDEX_ROLE);
            cameraItem->setData(selectedChanelIndex + 1, CHANEL_INDEX_ROLE);

            monitoringAreaInfoItem->setCheckable(true);
            monitoringAreaInfoItem->appendRow(cameraItem);
        }

        m_modelArea->appendRow(monitoringAreaInfoItem);
        ui->viewArea->setExpanded(m_modelArea->index(i, 0), true);
    }
}

void SelectChanelDlg::retranslateUI()
{
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);

    setWindowTitle(StringTable::Str_Select_chanel);
}


void SelectChanelDlg::areaItemDataChanged(QModelIndex topLeft, QModelIndex , QVector<int> )
{
    QStandardItem* standardItem = m_modelArea->itemFromIndex(topLeft);
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

