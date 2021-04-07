#include "areaselectdlg.h"
#include "ui_areaselectdlg.h"
#include "stringtable.h"

#include <QtWidgets>

AreaSelectDlg::AreaSelectDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AreaSelectDlg)
{
    ui->setupUi(this);
    retranslateUI();

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
}

AreaSelectDlg::~AreaSelectDlg()
{
    delete ui;
}

void AreaSelectDlg::setAreaInfo(QVector<MonitoringAreaInfo> areaInfo)
{
    m_areaInfo = areaInfo;

    ui->comboAreaName->clear();

    for(int i = 0; i < m_areaInfo.size(); i ++)
        ui->comboAreaName->addItem(m_areaInfo[i].areaName);
}

int AreaSelectDlg::selectedIndex()
{
    return ui->comboAreaName->currentIndex();
}

void AreaSelectDlg::slotOk()
{
    done(1);
}

void AreaSelectDlg::slotCancel()
{
    done(0);
}

void AreaSelectDlg::retranslateUI()
{
    ui->lblAreaName->setText(StringTable::Str_Monitoring_Area_Name_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);

    setWindowTitle(StringTable::Str_Select_Monitoring_Area);
}
