#include "monitoringserveillancesettingdlg.h"
#include "ui_monitoringserveillancesettingdlg.h"

#include "serverinfosocket.h"
#include "stringtable.h"
#include <QtWidgets>

MonitoringServeillanceSettingDlg::MonitoringServeillanceSettingDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonitoringServeillanceSettingDlg)
{
    ui->setupUi(this);
    retranslateUI();

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));

    connect(ui->m_chkFaceSize, SIGNAL(clicked()), this, SLOT(slotFaceSizeClicked()));
    connect(ui->m_chkThresold, SIGNAL(clicked()), this, SLOT(slotThresoldClicked()));
    connect(ui->m_chkBlackCandidateCount, SIGNAL(clicked()), this, SLOT(slotBlackCandidateCountClicked()));
    connect(ui->m_sldMinFaceSize, SIGNAL(valueChanged(int)), this, SLOT(slotMinFaceSizeChanged(int)));
    connect(ui->m_sldThresold, SIGNAL(valueChanged(int)), this, SLOT(slotThresoldChanged(int)));
    connect(ui->m_sldBlackCandidateCount, SIGNAL(valueChanged(int)), this, SLOT(slotBlackCandidateCountChanged(int)));
}

MonitoringServeillanceSettingDlg::~MonitoringServeillanceSettingDlg()
{
    delete ui;
}

void MonitoringServeillanceSettingDlg::setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex)
{
    m_serverInfoSocket = serverInfoSocket;
    m_chanelIndex = chanelIndex;

    if(m_chanelIndex < m_serverInfoSocket->serverInfo().ipCameraInfos.size())
        m_ipCameraInfo = m_serverInfoSocket->serverInfo().ipCameraInfos[m_chanelIndex];

    ui->m_chkFaceSize->setChecked(m_ipCameraInfo.chkFaceSize);
    if(ui->m_chkFaceSize->isChecked())
    {
        ui->m_lblMinFaceSize_Min->setEnabled(true);
        ui->m_lblCurFaceSize_Min->setEnabled(false);
        ui->m_lblMaxFaceSize_Min->setEnabled(true);
        ui->m_sldMinFaceSize->setEnabled(true);

        ui->m_sldMinFaceSize->setValue(m_ipCameraInfo.detectionFaceMinSize);

        slotMinFaceSizeChanged(m_ipCameraInfo.detectionFaceMinSize);
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

    ui->m_chkThresold->setChecked(m_ipCameraInfo.chkThreshold);
    if(ui->m_chkThresold->isChecked())
    {
        ui->m_lblCurThresold->setEnabled(false);
        ui->m_lblMaxThresold->setEnabled(true);
        ui->m_lblMinThresold->setEnabled(true);
        ui->m_sldThresold->setEnabled(true);
        ui->m_sldThresold->setValue(m_ipCameraInfo.identifyThreshold);
        slotThresoldChanged(m_ipCameraInfo.identifyThreshold);
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

    ui->m_chkBlackCandidateCount->setChecked(m_ipCameraInfo.chkBlackCandidateCount);
    if(ui->m_chkBlackCandidateCount->isChecked())
    {
        ui->m_lblCurBlackCandidateCount->setEnabled(false);
        ui->m_lblMaxBlackCandidateCount->setEnabled(true);
        ui->m_lblMinBlackCandidateCount->setEnabled(true);
        ui->m_sldBlackCandidateCount->setEnabled(true);
        ui->m_sldBlackCandidateCount->setValue(m_ipCameraInfo.blackCandidateCount);
        slotBlackCandidateCountChanged(m_ipCameraInfo.blackCandidateCount);
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

    connect(this, SIGNAL(valueChanged()), this, SLOT(slotValueChanged()));
}

void MonitoringServeillanceSettingDlg::slotOk()
{
    done(0);
}

void MonitoringServeillanceSettingDlg::slotFaceSizeClicked()
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

    emit valueChanged();
}

void MonitoringServeillanceSettingDlg::slotThresoldClicked()
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

    emit valueChanged();
}

void MonitoringServeillanceSettingDlg::slotBlackCandidateCountClicked()
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

    emit valueChanged();
}

void MonitoringServeillanceSettingDlg::slotThresoldChanged(int value)
{
    ui->m_lblCurThresold->setText(QString::number(value));

    emit valueChanged();
}

void MonitoringServeillanceSettingDlg::slotBlackCandidateCountChanged(int value)
{
    ui->m_lblCurBlackCandidateCount->setText(QString::number(value));

    emit valueChanged();
}

void MonitoringServeillanceSettingDlg::slotMinFaceSizeChanged(int value)
{
    QString text;
    text.sprintf("%d%", value);

    ui->m_lblCurFaceSize_Min->setText(text);
    ui->m_sldMinFaceSize->setValue(value);

    emit valueChanged();
}

void MonitoringServeillanceSettingDlg::slotValueChanged()
{
    IpCameraInfo cameraInfo;
    cameraInfo.chkFaceSize = ui->m_chkFaceSize->isChecked();
    cameraInfo.detectionFaceMinSize = ui->m_sldMinFaceSize->value();
    cameraInfo.chkThreshold = ui->m_chkThresold->isChecked();
    cameraInfo.identifyThreshold = ui->m_sldThresold->value();
    cameraInfo.chkBlackCandidateCount = ui->m_chkBlackCandidateCount->isChecked();
    cameraInfo.blackCandidateCount = ui->m_sldBlackCandidateCount->value();

    m_serverInfoSocket->setSurveillanceSetting(cameraInfo, m_chanelIndex);
}

void MonitoringServeillanceSettingDlg::retranslateUI()
{
    ui->groupSurveillanceSetting->setTitle(StringTable::Str_Surveillance_Setting);
    ui->m_chkFaceSize->setText(StringTable::Str_Min_Face_Size);
    ui->m_chkThresold->setText(StringTable::Str_Threshold);
    ui->m_chkBlackCandidateCount->setText(StringTable::Str_BlackCandidateCount);
    ui->btnOk->setText(StringTable::Str_Ok);

    setWindowTitle(StringTable::Str_Surveillance_Setting);
}
