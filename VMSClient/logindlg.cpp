#include "logindlg.h"
#include "ui_logindlg.h"

#include "clientbase.h"
#include "stringtable.h"

#include <QMessageBox>

LoginDlg::LoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlg)
{
    ui->setupUi(this);

    ui->editInput->setEchoMode(QLineEdit::Password);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    retranslateUI();
}

LoginDlg::~LoginDlg()
{
    delete ui;
}

void LoginDlg::slotOk()
{
    if(ui->editInput->text() != readPass())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Fail_to_match_password);
        ui->editInput->setText("");
        ui->editInput->setFocus();
        return;
    }

    done(1);
}

void LoginDlg::slotCancel()
{
    done(0);
}


void LoginDlg::retranslateUI()
{
    setWindowTitle(StringTable::Str_Log_In);
    ui->lblInput->setText(StringTable::Str_Confirm_Password_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);
}
