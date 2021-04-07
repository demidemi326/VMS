#include "monitoringipcamerasettngdlg.h"
#include "ui_monitoringipcamerasettngdlg.h"
#include "base.h"
#include "selectcameradlg.h"

#include "stringtable.h"

#include <QtWidgets>

MonitoringIpCameraSettngDlg::MonitoringIpCameraSettngDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonitoringIpCameraSettngDlg)
{
    ui->setupUi(this);
    retranslateUI();

    QRegExp ipRegExp("([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))");
    QValidator *ipValidator = new QRegExpValidator(ipRegExp, ui->editIpAddress);
    ui->editIpAddress->setValidator(ipValidator);

    ui->editIpAddress->setEnabled(false);
    ui->editVideoSource->setEnabled(false);
    ui->editStreamUri->hide();
    ui->lblStreamUri->hide();

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(ui->btnClear,SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(ui->btnSelectCamera, SIGNAL(clicked()), this, SLOT(slotSelectCamera()));
}

MonitoringIpCameraSettngDlg::~MonitoringIpCameraSettngDlg()
{
    delete ui;
}

void MonitoringIpCameraSettngDlg::setChanelInfo(ServerInfo serverInfo, int chanelIndex)
{
    m_chanelIndex = chanelIndex;
    m_serverInfo = serverInfo;

    m_ipCameraInfo = serverInfo.ipCameraInfos[chanelIndex];

    ui->editIpAddress->setText(m_ipCameraInfo.ipAddress);
    ui->editVideoSource->setText(m_ipCameraInfo.videoSource);
    ui->editStreamUri->setText(m_ipCameraInfo.streamuri);
}


IpCameraInfo MonitoringIpCameraSettngDlg::ipCameraInfo()
{
    return m_ipCameraInfo;
}

void MonitoringIpCameraSettngDlg::slotOk()
{
//    QStringList subAddressList = ui->editIpAddress->text().split(".");
//    if(subAddressList.size() != 4)
//    {
//        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Invalid_ip_address);
//        ui->editIpAddress->setFocus();
//        return;
//    }

//    if(ui->editPortNum->text().isEmpty())
//    {
//        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_port_num);
//        ui->editPortNum->setFocus();
//        return;
//    }

//    if(ui->editUserName->text().isEmpty())
//    {
//        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_user_name);
//        ui->editUserName->setFocus();
//        return;
//    }

//    if(ui->editPassword->text().isEmpty())
//    {
//        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_password);
//        ui->editPassword->setFocus();
//        return;
//    }
#ifndef _DUP_IP_
    for(int i = 0; i < m_serverInfo.ipCameraInfos.size(); i ++)
    {
        if(m_chanelIndex != i && m_serverInfo.ipCameraInfos[i].ipAddress == ui->editIpAddress->text())
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Duplicated_ip_adress);
            ui->editIpAddress->setFocus();
            return;
        }
    }
#endif

    m_ipCameraInfo.ipAddress = ui->editIpAddress->text();
    m_ipCameraInfo.videoSource = ui->editVideoSource->text();
    m_ipCameraInfo.streamuri = ui->editStreamUri->text();

    done(1);
}

void MonitoringIpCameraSettngDlg::slotCancel()
{
    done(0);
}


void MonitoringIpCameraSettngDlg::retranslateUI()
{
    ui->groupCameraSetting->setTitle(StringTable::Str_Camera_Setting);
    ui->lblIpAddress->setText(StringTable::Str_IP_Address_);
    ui->lblVideoSource->setText(StringTable::Str_VideoSource + ":");
    ui->lblStreamUri->setText(StringTable::Str_Password_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);
    ui->btnClear->setText(StringTable::Str_Clear);
    ui->btnSelectCamera->setText(StringTable::Str_Select_Camera);

    setWindowTitle(StringTable::Str_Camera_Setting);

}

void MonitoringIpCameraSettngDlg::slotClear()
{
    if(QMessageBox::question(this, StringTable::Str_Question, StringTable::Str_Are_you_sure_to_delete_all_info ,QMessageBox::Yes | QMessageBox::No , QMessageBox::No) != QMessageBox::Yes)
        return;

    ui->editIpAddress->setText("");
    ui->editStreamUri->setText("");
    ui->editVideoSource->setText("");
}

void MonitoringIpCameraSettngDlg::slotSelectCamera()
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
