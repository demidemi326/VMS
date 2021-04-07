#include "modifypass.h"
#include "ui_modifypass.h"
#include "clientbase.h"
#include "stringtable.h"

#include <QMessageBox>

ModifyPass::ModifyPass(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyPass)
{
    ui->setupUi(this);

    ui->editCur->setEchoMode(QLineEdit::Password);
    ui->editNew->setEchoMode(QLineEdit::Password);
    ui->editConfirm->setEchoMode(QLineEdit::Password);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    retranslateUI();
}

ModifyPass::~ModifyPass()
{
    delete ui;
}

void ModifyPass::slotOk()
{
    QString strCurPass = readPass();
    if(strCurPass != ui->editCur->text())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Fail_to_match_old_password);

        ui->editCur->setText("");
        ui->editNew->setText("");
        ui->editConfirm->setText("");
        ui->editCur->setFocus();
        return;
    }

    if(ui->editNew->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_a_new_password);
        ui->editNew->setFocus();
        return;
    }

    if(ui->editConfirm->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_a_confirm_password);
        ui->editConfirm->setFocus();
        return;
    }

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

void ModifyPass::slotCancel()
{
    done(0);
}


void ModifyPass::retranslateUI()
{
    setWindowTitle(StringTable::Str_Change_Password);
    ui->lblCur->setText(StringTable::Str_Old_Password_);
    ui->lblNew->setText(StringTable::Str_New_assword_);
    ui->lblConfirm->setText(StringTable::Str_Confirm_Password_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);
}
