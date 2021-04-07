#include "ipcamerasettingdlg.h"
#include "ui_ipcamerasettingdlg.h"
#include "stringtable.h"
#include "base.h"
#include "selectcameradlg.h"

#include <QtWidgets>

IpCameraSettingDlg::IpCameraSettingDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IpCameraSettingDlg)
{
    ui->setupUi(this);
    retranslateUI();

    m_model = new QStandardItemModel(0, 1);
    m_model->setHeaderData(0, Qt::Horizontal, StringTable::Str_IP_Camera);

    m_selectionModel = new QItemSelectionModel(m_model);

    ui->viewIPCameras->setModel(m_model);
    ui->viewIPCameras->setSelectionModel(m_selectionModel);
    ui->viewIPCameras->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewIPCameras->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewIPCameras->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewIPCameras->setAlternatingRowColors(true);

    m_rootItem = new QStandardItem;
    m_rootItem->setIcon(QIcon(QPixmap(":/images/machine.png")));
    m_model->appendRow(m_rootItem);

    QRegExp ipRegExp("([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))");
    QValidator *ipValidator = new QRegExpValidator(ipRegExp, ui->editIpAddress);
    ui->editIpAddress->setValidator(ipValidator);

    selectIpCamera(-1);

    connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotIPCameraSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(ui->btnApply, SIGNAL(clicked()), this, SLOT(slotApply()));

    connect(ui->editIpAddress, SIGNAL(textChanged(QString)), this, SLOT(slotChangedIpCameraSetting(QString)));
    connect(ui->editVideoSource, SIGNAL(textChanged(QString)), this, SLOT(slotChangedIpCameraSetting(QString)));
    connect(ui->editStreamUri, SIGNAL(textChanged(QString)), this, SLOT(slotChangedIpCameraSetting(QString)));

    connect(ui->m_chkFaceSize, SIGNAL(clicked()), this, SLOT(slotFaceSizeClicked()));
    connect(ui->m_chkThresold, SIGNAL(clicked()), this, SLOT(slotThresoldClicked()));
    connect(ui->m_chkBlackCandidateCount, SIGNAL(clicked()), this, SLOT(slotBlackCandidateCountClicked()));
    connect(ui->m_sldThresold, SIGNAL(valueChanged(int)), this, SLOT(slotThresoldChanged(int)));
    connect(ui->m_sldBlackCandidateCount, SIGNAL(valueChanged(int)), this, SLOT(slotBlackCandidateCountChanged(int)));
    connect(ui->m_sldMinFaceSize, SIGNAL(valueChanged(int)), this, SLOT(slotMinFaceSizeChanged(int)));

    connect(ui->btnClear, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(ui->btnSeletCamera, SIGNAL(clicked()), this, SLOT(slotSelectCamera()));

    ui->editIpAddress->setEnabled(false);
    ui->editVideoSource->setEnabled(false);
    ui->editStreamUri->hide();
    ui->lblStreamUri->hide();
}

IpCameraSettingDlg::~IpCameraSettingDlg()
{
    delete ui;
}

void IpCameraSettingDlg::setServerInfo(ServerInfo serverInfo)
{
    m_serverInfo = serverInfo;

    m_rootItem->setText(serverInfo.serverName);

    m_rootItem->removeRows(0, m_rootItem->rowCount());
    for(int i = 0; i < m_serverInfo.ipCameraInfos.size(); i ++)
    {
        QString chanelIndexStr;
        chanelIndexStr.sprintf(" (%d)", i + 1);

        QStandardItem* chanelItem = new QStandardItem;
        chanelItem->setText(StringTable::Str_Chanel + chanelIndexStr);
        chanelItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));

        m_rootItem->appendRow(chanelItem);
    }
    ui->viewIPCameras->setExpanded(m_model->index(0, 0), true);
}

QVector<IpCameraInfo> IpCameraSettingDlg::ipCameraInfos()
{
    return m_serverInfo.ipCameraInfos;
}

void IpCameraSettingDlg::slotIPCameraSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    changeSelectItem();
}

void IpCameraSettingDlg::slotApply()
{
    if(m_selectedIndex < 0)
        return;

#ifndef _DUP_IP_
    for(int i = 0; i < m_serverInfo.ipCameraInfos.size(); i ++)
    {
        if(m_selectedIndex != i && m_serverInfo.ipCameraInfos[i].ipAddress == ui->editIpAddress->text())
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Duplicated_ip_adress);
            ui->editIpAddress->setFocus();
            return;
        }
    }
#endif

    m_serverInfo.ipCameraInfos[m_selectedIndex].ipAddress = ui->editIpAddress->text();
    m_serverInfo.ipCameraInfos[m_selectedIndex].videoSource = ui->editVideoSource->text();
    m_serverInfo.ipCameraInfos[m_selectedIndex].streamuri = ui->editStreamUri->text();

    m_serverInfo.ipCameraInfos[m_selectedIndex].chkFaceSize = ui->m_chkFaceSize->isChecked();
    m_serverInfo.ipCameraInfos[m_selectedIndex].detectionFaceMinSize = ui->m_sldMinFaceSize->value();
    m_serverInfo.ipCameraInfos[m_selectedIndex].chkThreshold = ui->m_chkThresold->isChecked();
    m_serverInfo.ipCameraInfos[m_selectedIndex].identifyThreshold = ui->m_sldThresold->value();
    m_serverInfo.ipCameraInfos[m_selectedIndex].chkBlackCandidateCount = ui->m_chkBlackCandidateCount->isChecked();
    m_serverInfo.ipCameraInfos[m_selectedIndex].blackCandidateCount = ui->m_sldBlackCandidateCount->value();

    ui->btnApply->setEnabled(false);
}

void IpCameraSettingDlg::slotChangedIpCameraSetting(QString)
{
    ui->btnApply->setEnabled(true);
}

void IpCameraSettingDlg::changeSelectItem()
{
    if(!m_selectionModel->hasSelection())
    {
        selectIpCamera(-1);

        return;
    }

    QModelIndex index = m_selectionModel->selection().indexes().at(0);
    QString selectedName = m_model->data(index).toString();

    int exist = -1;
    for(int i = 0; i < m_serverInfo.ipCameraInfos.size(); i ++)
    {
        QString chanelIndexStr;
        chanelIndexStr.sprintf(" (%d)", i + 1);
        if(selectedName == StringTable::Str_Chanel + chanelIndexStr)
        {
            exist = i;
            break;
        }
    }

    selectIpCamera(exist);

    m_selectedIndex = exist;
    if(exist < 0)
    {
        ui->editIpAddress->setText("");
        ui->editVideoSource->setText("");
        ui->editStreamUri->setText("");
        ui->btnApply->setEnabled(false);
        ui->btnClear->setEnabled(false);

        return;
    }

    ui->editIpAddress->setText(m_serverInfo.ipCameraInfos[exist].ipAddress);
    ui->editVideoSource->setText(m_serverInfo.ipCameraInfos[exist].videoSource);
    ui->editStreamUri->setText(m_serverInfo.ipCameraInfos[exist].streamuri);

    ui->m_chkFaceSize->setChecked(m_serverInfo.ipCameraInfos[exist].chkFaceSize);
    if(ui->m_chkFaceSize->isChecked())
    {
        ui->m_lblMinFaceSize_Min->setEnabled(true);
        ui->m_lblCurFaceSize_Min->setEnabled(false);
        ui->m_lblMaxFaceSize_Min->setEnabled(true);
        ui->m_sldMinFaceSize->setEnabled(true);

        ui->m_sldMinFaceSize->setValue(m_serverInfo.ipCameraInfos[exist].detectionFaceMinSize);

        slotMinFaceSizeChanged(m_serverInfo.ipCameraInfos[exist].detectionFaceMinSize);
    }
    else
    {
        ui->m_lblMinFaceSize_Min->setEnabled(false);
        ui->m_lblCurFaceSize_Min->setEnabled(false);
        ui->m_lblMaxFaceSize_Min->setEnabled(false);
        ui->m_sldMinFaceSize->setEnabled(false);

        ui->m_sldMinFaceSize->setValue(DEFAULT_MIN_FACESIZE);
        slotMinFaceSizeChanged(DEFAULT_MIN_FACESIZE);
    }

    ui->m_chkThresold->setChecked(m_serverInfo.ipCameraInfos[exist].chkThreshold);
    if(ui->m_chkThresold->isChecked())
    {
        ui->m_lblCurThresold->setEnabled(false);
        ui->m_lblMaxThresold->setEnabled(true);
        ui->m_lblMinThresold->setEnabled(true);
        ui->m_sldThresold->setEnabled(true);
        ui->m_sldThresold->setValue(m_serverInfo.ipCameraInfos[exist].identifyThreshold);
        slotThresoldChanged(m_serverInfo.ipCameraInfos[exist].identifyThreshold);
    }
    else
    {
        ui->m_lblCurThresold->setEnabled(false);
        ui->m_lblMaxThresold->setEnabled(false);
        ui->m_lblMinThresold->setEnabled(false);
        ui->m_sldThresold->setEnabled(false);

        ui->m_sldThresold->setValue(DEFAULT_IDENTIFY_THRESHOLD);

        slotThresoldChanged(DEFAULT_IDENTIFY_THRESHOLD);
    }

    ui->m_chkBlackCandidateCount->setChecked(m_serverInfo.ipCameraInfos[exist].chkBlackCandidateCount);
    if(ui->m_chkBlackCandidateCount->isChecked())
    {
        ui->m_lblCurBlackCandidateCount->setEnabled(false);
        ui->m_lblMaxBlackCandidateCount->setEnabled(true);
        ui->m_lblMinBlackCandidateCount->setEnabled(true);
        ui->m_sldBlackCandidateCount->setEnabled(true);
        ui->m_sldBlackCandidateCount->setValue(m_serverInfo.ipCameraInfos[exist].blackCandidateCount);
        slotBlackCandidateCountChanged(m_serverInfo.ipCameraInfos[exist].blackCandidateCount);
    }
    else
    {
        ui->m_lblCurBlackCandidateCount->setEnabled(false);
        ui->m_lblMaxBlackCandidateCount->setEnabled(false);
        ui->m_lblMinBlackCandidateCount->setEnabled(false);
        ui->m_sldBlackCandidateCount->setEnabled(false);

        ui->m_sldBlackCandidateCount->setValue(DEFAULT_BLACK_CANDIDATE_COUNT);

        slotBlackCandidateCountChanged(DEFAULT_BLACK_CANDIDATE_COUNT);
    }

    ui->btnApply->setEnabled(false);
    ui->btnClear->setEnabled(true);
}

void IpCameraSettingDlg::selectIpCamera(int index)
{
    if(index < 0)
    {
        m_selectedIndex = -1;

        ui->editIpAddress->setText("");
        ui->editVideoSource->setText("");
        ui->editStreamUri->setText("");

        ui->btnApply->setEnabled(false);
        ui->btnClear->setEnabled(false);


        ui->groupCameraSetting->setEnabled(false);
        ui->groupSurveillanceSetting->setEnabled(false);
    }
    else
    {
        m_selectedIndex = index;
        ui->editIpAddress->setText(m_serverInfo.ipCameraInfos[m_selectedIndex].ipAddress);
        ui->editVideoSource->setText(m_serverInfo.ipCameraInfos[m_selectedIndex].videoSource);
        ui->editStreamUri->setText(m_serverInfo.ipCameraInfos[m_selectedIndex].streamuri);

        ui->editIpAddress->setEnabled(false);
        ui->editVideoSource->setEnabled(false);
        ui->editStreamUri->setEnabled(false);

        ui->btnApply->setEnabled(false);
        ui->btnClear->setEnabled(true);

        ui->groupCameraSetting->setEnabled(false);
        ui->groupSurveillanceSetting->setEnabled(true);
    }
}


void IpCameraSettingDlg::slotMinFaceSizeChanged(int value)
{
    QString text;
    text.sprintf("%d%", value);

    ui->m_lblCurFaceSize_Min->setText(text);
    ui->m_sldMinFaceSize->setValue(value);

    ui->btnApply->setEnabled(true);
}


void IpCameraSettingDlg::slotThresoldChanged(int value)
{
    ui->m_lblCurThresold->setText(QString::number(value));

    ui->btnApply->setEnabled(true);
}

void IpCameraSettingDlg::slotBlackCandidateCountChanged(int value)
{
    ui->m_lblCurBlackCandidateCount->setText(QString::number(value));

    ui->btnApply->setEnabled(true);
}

void IpCameraSettingDlg::slotFaceSizeClicked()
{
    if(ui->m_chkFaceSize->isChecked())
    {
        ui->m_lblMinFaceSize_Min->setEnabled(true);
        ui->m_lblCurFaceSize_Min->setEnabled(false);
        ui->m_lblMaxFaceSize_Min->setEnabled(true);
        ui->m_sldMinFaceSize->setEnabled(true);
    }
    else
    {
        ui->m_lblMinFaceSize_Min->setEnabled(false);
        ui->m_lblCurFaceSize_Min->setEnabled(false);
        ui->m_lblMaxFaceSize_Min->setEnabled(false);
        ui->m_sldMinFaceSize->setEnabled(false);

        ui->m_sldMinFaceSize->setValue(DEFAULT_MIN_FACESIZE);
        slotMinFaceSizeChanged(DEFAULT_MIN_FACESIZE);
    }

    ui->btnApply->setEnabled(true);
}

void IpCameraSettingDlg::slotThresoldClicked()
{
    if(ui->m_chkThresold->isChecked())
    {
        ui->m_lblCurThresold->setEnabled(false);
        ui->m_lblMaxThresold->setEnabled(true);
        ui->m_lblMinThresold->setEnabled(true);
        ui->m_sldThresold->setEnabled(true);
    }
    else
    {
        ui->m_lblCurThresold->setEnabled(false);
        ui->m_lblMaxThresold->setEnabled(false);
        ui->m_lblMinThresold->setEnabled(false);
        ui->m_sldThresold->setEnabled(false);

        ui->m_sldThresold->setValue(DEFAULT_IDENTIFY_THRESHOLD);

        slotThresoldChanged(DEFAULT_IDENTIFY_THRESHOLD);
    }

    ui->btnApply->setEnabled(true);
}

void IpCameraSettingDlg::slotBlackCandidateCountClicked()
{
    if(ui->m_chkBlackCandidateCount->isChecked())
    {
        ui->m_lblCurBlackCandidateCount->setEnabled(false);
        ui->m_lblMaxBlackCandidateCount->setEnabled(true);
        ui->m_lblMinBlackCandidateCount->setEnabled(true);
        ui->m_sldBlackCandidateCount->setEnabled(true);
    }
    else
    {
        ui->m_lblCurBlackCandidateCount->setEnabled(false);
        ui->m_lblMaxBlackCandidateCount->setEnabled(false);
        ui->m_lblMinBlackCandidateCount->setEnabled(false);
        ui->m_sldBlackCandidateCount->setEnabled(false);

        ui->m_sldBlackCandidateCount->setValue(DEFAULT_BLACK_CANDIDATE_COUNT);

        slotBlackCandidateCountChanged(DEFAULT_BLACK_CANDIDATE_COUNT);
    }

    ui->btnApply->setEnabled(true);
}

void IpCameraSettingDlg::retranslateUI()
{
    ui->groupCameraSetting->setTitle(StringTable::Str_Camera_Setting);
    ui->groupSurveillanceSetting->setTitle(StringTable::Str_Surveillance_Setting);
    ui->lblIpAddress->setText(StringTable::Str_IP_Address_);
    ui->lblVideoSource->setText(StringTable::Str_VideoSource + ":");
    ui->lblStreamUri->setText(StringTable::Str_Password_);
    ui->m_chkFaceSize->setText(StringTable::Str_Min_Face_Size);
    ui->m_chkThresold->setText(StringTable::Str_Threshold);
    ui->m_chkBlackCandidateCount->setText(StringTable::Str_BlackCandidateCount);
    ui->btnApply->setText(StringTable::Str_Apply);
    ui->btnClear->setText(StringTable::Str_Clear);
    ui->btnSeletCamera->setText(StringTable::Str_Select_Camera);

    setWindowTitle(StringTable::Str_Chanel_Setting);

}

void IpCameraSettingDlg::slotClear()
{
    if(QMessageBox::question(this, StringTable::Str_Question, StringTable::Str_Are_you_sure_to_delete_all_info ,QMessageBox::Yes | QMessageBox::No , QMessageBox::No) != QMessageBox::Yes)
        return;

    ui->editIpAddress->setText("");
    ui->editStreamUri->setText("");
    ui->editVideoSource->setText("");

    slotApply();
}

void IpCameraSettingDlg::slotSelectCamera()
{
    SelectCameraDlg dlg;
    if(dlg.exec())
    {
        QHash<QString, QString> cameraInfos = dlg.cameraInfos();
        ui->editIpAddress->setText(cameraInfos["ip"]);
        ui->editVideoSource->setText(cameraInfos["videosource"]);
        ui->editStreamUri->setText(cameraInfos["streamuri"]);
    }
}

