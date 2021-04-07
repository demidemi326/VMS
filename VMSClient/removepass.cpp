#include "removepass.h"
#include "ui_removepass.h"
#include "stringtable.h"

#include "clientbase.h"
#include <QMessageBox>

RemovePass::RemovePass(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemovePass)
{
    ui->setupUi(this);

    ui->editCur->setEchoMode(QLineEdit::Password);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    retranslateUI();
}

RemovePass::~RemovePass()
{
    delete ui;
}

void RemovePass::slotOk()
{
    QString strCurPass = readPass();
    if(strCurPass != ui->editCur->text())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Fail_to_match_password);
        ui->editCur->setText("");
        ui->editCur->setFocus();
        return;
    }

    writePass("");

    done(1);
}

void RemovePass::slotCancel()
{
    done(0);
}



void RemovePass::retranslateUI()
{
    setWindowTitle(StringTable::Str_Remove_Password);
    ui->lblCur->setText(StringTable::Str_Old_Password_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);
}
