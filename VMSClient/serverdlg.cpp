#include "serverdlg.h"
#include "ui_serverdlg.h"
#include "stringtable.h"
#include "socketbase.h"
#include "serverinfosocket.h"

#include <QtWidgets>

ServerDlg::ServerDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDlg)
{
    ui->setupUi(this);
    retranslateUI();

    QRegExp ipRegExp("([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))");
    QValidator *ipValidator = new QRegExpValidator(ipRegExp, ui->editIpAddress);
    ui->editIpAddress->setValidator(ipValidator);

    QRegExp portRegExp("([0-9])([0-9])([0-9])([0-9])([0-9])");
    QValidator *portValidator = new QRegExpValidator(portRegExp, ui->editPort);
    ui->editPort->setValidator(portValidator);

    ui->editPort->setText(QString::number(VMS_SERVER_PORT));

    ui->editPassword->setEchoMode(QLineEdit::Password);

    m_editIndex = -1;

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
}

ServerDlg::~ServerDlg()
{
    delete ui;
}

void ServerDlg::setServerInfo(QVector<ServerInfoSocket*> serverInfoSockets)
{
    m_serverInfoSockets = serverInfoSockets;
}

void ServerDlg::setEditServerInfo(QVector<ServerInfoSocket*> serverInfos, int index)
{
    m_serverInfoSockets = serverInfos;
    m_editIndex = index;

    ServerInfo serverInfo = serverInfos[index]->serverInfo();
    ui->editMachineName->setText(serverInfo.serverName);
    ui->editIpAddress->setText(serverInfo.ipAddress);
    ui->editPort->setText(QString::number(serverInfo.port));
    ui->editUserName->setText(serverInfo.userName);
    ui->editPassword->setText(serverInfo.password);
}

ServerInfo ServerDlg::serverInfo()
{
    ServerInfo serverInfo;
    serverInfo.serverName = ui->editMachineName->text();
    serverInfo.ipAddress = ui->editIpAddress->text();
    serverInfo.port = ui->editPort->text().toInt();
    serverInfo.userName = ui->editUserName->text();
    serverInfo.password = ui->editPassword->text();
    return serverInfo;
}

void ServerDlg::slotOk()
{
    if(ui->editMachineName->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_machine_name);
        ui->editMachineName->setFocus();
        return;
    }

    QStringList subAddressList = ui->editIpAddress->text().split(".");
    if(subAddressList.size() != 4)
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Invalid_ip_address);
        ui->editIpAddress->setFocus();
        return;
    }

    if(ui->editPort->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_port);
        ui->editPort->setFocus();
        return;
    }

    if(ui->editUserName->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_user_name);
        ui->editUserName->setFocus();
        return;
    }

    for(int i = 0; i < m_serverInfoSockets.size(); i ++)
    {
        ServerInfo serverInfo = m_serverInfoSockets[i]->serverInfo();
        if(serverInfo.serverName == ui->editMachineName->text() && i != m_editIndex)
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Duplicated_machine_name);
            ui->editMachineName->setFocus();
            return;
        }
        else if(serverInfo.ipAddress == ui->editIpAddress->text() && i != m_editIndex)
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Duplicated_server);
            ui->editMachineName->setFocus();
            return;
        }
    }

    done(1);
}

void ServerDlg::slotCancel()
{
    done(0);
}

void ServerDlg::retranslateUI()
{
    ui->lblMachineName->setText(StringTable::Str_Machine_Name_);
    ui->lblIpAddress->setText(StringTable::Str_IP_Address_);
    ui->lblPortNum->setText(StringTable::Str_Port_Num_);
    ui->lblUserName->setText(StringTable::Str_User_Name);
    ui->lblPassword->setText(StringTable::Str_Password_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);

    setWindowTitle(StringTable::Str_Machine);
}
