#include "activationdlg.h"
#include "ui_activationdlg.h"
#include "frengine.h"
#include "servicebase.h"

#include <QtWidgets>

ActivationDlg::ActivationDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActivationDlg)
{
    ui->setupUi(this);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    char szHWID[1024] = { 0 };
    GetCurrentHWID(szHWID, sizeof(szHWID));

    ui->editProduct->setText(QString::fromUtf8(szHWID));

    m_activated = -1;
}

ActivationDlg::~ActivationDlg()
{
    delete ui;
}

enum VMProtectSerialStateFlags
{
    SERIAL_STATE_FLAG_CORRUPTED			= 0x00000001,
    SERIAL_STATE_FLAG_INVALID			= 0x00000002,
    SERIAL_STATE_FLAG_BLACKLISTED		= 0x00000004,
    SERIAL_STATE_FLAG_DATE_EXPIRED		= 0x00000008,
    SERIAL_STATE_FLAG_RUNNING_TIME_OVER	= 0x00000010,
    SERIAL_STATE_FLAG_BAD_HWID			= 0x00000020,
    SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED	= 0x00000040,
};

#define PRINT_HELPER(msg, state, flag) if (state & flag) sprintf(msg, "%s ", #flag)
QString print_state(int state)
{
    QString strMsg;
    char szMsg[1024] = { 0 };
    PRINT_HELPER(szMsg, state, SERIAL_STATE_FLAG_CORRUPTED);
    PRINT_HELPER(szMsg, state, SERIAL_STATE_FLAG_INVALID);
    PRINT_HELPER(szMsg, state, SERIAL_STATE_FLAG_BLACKLISTED);
    PRINT_HELPER(szMsg, state, SERIAL_STATE_FLAG_DATE_EXPIRED);
    PRINT_HELPER(szMsg, state, SERIAL_STATE_FLAG_RUNNING_TIME_OVER);
    PRINT_HELPER(szMsg, state, SERIAL_STATE_FLAG_BAD_HWID);
    PRINT_HELPER(szMsg, state, SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED);

    strMsg = szMsg;

    return strMsg;
}

void ActivationDlg::slotOk()
{
    if(ui->editSerial->toPlainText().isEmpty())
    {
        QMessageBox::information(this, tr("Warning"), tr("Failure activation!"));
        ui->editSerial->setFocus();
        return;
    }

    QString serialStr = ui->editSerial->toPlainText();

    int ret = SetActivation(serialStr.toUtf8().data());
    m_activated = !ret;
    if(ret == 0)
    {
        QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
        setting.setValue("activation key", serialStr);
        QMessageBox::information(this, tr("Information"), tr("Successful activation!"));
    }
    else
    {
        QString errorStr = print_state(ret);
        QMessageBox::warning(this, tr("Warning"), tr("Failure activation!"));
        ui->editSerial->setFocus();
        return;
    }

    done(m_activated);
}

void ActivationDlg::slotCancel()
{
    done(0);
}

