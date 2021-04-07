#include "blackresultdetaildlg.h"
#include "ui_blackresultdetaildlg.h"

BlackResultDetailDlg::BlackResultDetailDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlackResultDetailDlg)
{
    ui->setupUi(this);
}

BlackResultDetailDlg::~BlackResultDetailDlg()
{
    delete ui;
}
