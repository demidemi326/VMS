#include "ipcameraonesettingdlg.h"
#include "ui_ipcameraonesettingdlg.h"
#include "stringtable.h"

#include <QtWidgets>

IpCameraOneSettingDlg::IpCameraOneSettingDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IpCameraOneSettingDlg)
{
    ui->setupUi(this);

    QRegExp ipRegExp("([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))");
    QRegExp portRegExp("([0-9])([0-9])([0-9])([0-9])([0-9])");
    QValidator *ipValidator = new QRegExpValidator(ipRegExp, ui->editIpAddress);
    QValidator *portValidator = new QRegExpValidator(portRegExp, ui->editPortNum);
    ui->editIpAddress->setValidator(ipValidator);
    ui->editPortNum->setValidator(portValidator);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    connect(ui->m_chkFaceSize, SIGNAL(clicked()), this, SLOT(slotFaceSizeClicked()));
    connect(ui->m_chkThresold, SIGNAL(clicked()), this, SLOT(slotThresoldClicked()));
    connect(ui->m_sldThresold, SIGNAL(valueChanged(int)), this, SLOT(slotThresoldChanged(int)));
    connect(ui->m_sldMinFaceSize, SIGNAL(valueChanged(int)), this, SLOT(slotMinFaceSizeChanged(int)));
}

IpCameraOneSettingDlg::~IpCameraOneSettingDlg()
{
    delete ui;
}

void IpCameraOneSettingDlg::setIpCameraInfo(IpCameraInfo ipCameraInfo)
{
    m_ipCameraInfo = ipCameraInfo;

    ui->editIpAddress->setText(m_ipCameraInfo.ipAddress);
    ui->editPortNum->setText(QString::number(m_ipCameraInfo.portNum));
    ui->editUserName->setText(m_ipCameraInfo.videoSource);
    ui->editPassword->setText(m_ipCameraInfo.streamuri);

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

    ui->m_chkThresold->setChecked(ipCameraInfo.chkThreshold);
    if(ui->m_chkThresold->isChecked())
    {
        ui->m_lblCurThresold->setEnabled(false);
        ui->m_lblMaxThresold->setEnabled(true);
        ui->m_lblMinThresold->setEnabled(true);
        ui->m_sldThresold->setEnabled(true);
        ui->m_sldThresold->setValue(ipCameraInfo.identifyThreshold);
        slotThresoldChanged(ipCameraInfo.identifyThreshold);
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
}

IpCameraInfo IpCameraOneSettingDlg::ipCameraInfo()
{
    return m_ipCameraInfo;
}

void IpCameraOneSettingDlg::slotOk()
{
    QStringList subAddressList = ui->editIpAddress->text().split(".");
    if(subAddressList.size() != 4)
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Invalid_ip_address);
        ui->editIpAddress->setFocus();
        return;
    }

    if(ui->editPortNum->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_port_num);
        ui->editPortNum->setFocus();
        return;
    }

    if(ui->editUserName->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_user_name);
        ui->editUserName->setFocus();
        return;
    }

    if(ui->editPassword->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_password);
        ui->editPassword->setFocus();
        return;
    }

    m_ipCameraInfo.ipAddress = ui->editIpAddress->text();
    m_ipCameraInfo.portNum = ui->editPortNum->text().toInt();
    m_ipCameraInfo.videoSource = ui->editUserName->text();
    m_ipCameraInfo.streamuri = ui->editPassword->text();

    m_ipCameraInfo.chkFaceSize = ui->m_chkFaceSize->isChecked();
    m_ipCameraInfo.detectionFaceMinSize = ui->m_sldMinFaceSize->value();
    m_ipCameraInfo.chkThreshold = ui->m_chkThresold->isChecked();
    m_ipCameraInfo.identifyThreshold = ui->m_sldThresold->value();

    done(1);
}

void IpCameraOneSettingDlg::slotCancel()
{
    done(0);
}


void IpCameraOneSettingDlg::slotMinFaceSizeChanged(int value)
{
    QString text;
    text.sprintf("%d%", value);

    ui->m_lblCurFaceSize_Min->setText(text);
    ui->m_sldMinFaceSize->setValue(value);
}


void IpCameraOneSettingDlg::slotThresoldChanged(int value)
{
    ui->m_lblCurThresold->setText(QString::number(value));
}

void IpCameraOneSettingDlg::slotFaceSizeClicked()
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
}

void IpCameraOneSettingDlg::slotThresoldClicked()
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
}
