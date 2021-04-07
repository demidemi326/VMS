#include "newpass.h"
#include "ui_newpass.h"
#include "stringtable.h"

#include "clientbase.h"

#include <QMessageBox>

NewPass::NewPass(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewPass)
{
    ui->setupUi(this);

    ui->editNew->setEchoMode(QLineEdit::Password);
    ui->editConfirm->setEchoMode(QLineEdit::Password);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    retranslateUI();
}

NewPass::~NewPass()
{
    delete ui;
}

void NewPass::slotOk()
{
    if(ui->editNew->text() != ui->editConfirm->text())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_The_passwords_you_typed_do_not_match);
        ui->editNew->setText("");
        ui->editConfirm->setText("");
        ui->editNew->setFocus();

        return;
    }

    QString srNewPass = ui->editNew->text();
    writePass(srNewPass);

    done(1);
}

void NewPass::slotCancel()
{
    done(0);
}

void NewPass::retranslateUI()
{
    setWindowTitle(StringTable::Str_New_assword);
    ui->lblNew->setText(StringTable::Str_New_assword_);
    ui->lblConfirm->setText(StringTable::Str_Confirm_Password_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);
}
