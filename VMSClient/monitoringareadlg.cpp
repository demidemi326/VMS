#include "monitoringareadlg.h"
#include "ui_monitoringareadlg.h"
#include "clientbase.h"
#include "stringtable.h"

#include <QtWidgets>

MonitoringAreaDlg::MonitoringAreaDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonitoringAreaDlg)
{
    ui->setupUi(this);
    retranslateUI();

    m_editIndex = -1;

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
}

MonitoringAreaDlg::~MonitoringAreaDlg()
{
    delete ui;
}

void MonitoringAreaDlg::setMonitoringAreaInfos(QVector<MonitoringAreaInfo> monitoringAreaInfos, int editIndex)
{
    m_monitoringAreaInfos = monitoringAreaInfos;
    m_editIndex = editIndex;

    if(m_editIndex >= 0)
        ui->editMonitoringAreaName->setText(m_monitoringAreaInfos[m_editIndex].areaName);
}

MonitoringAreaInfo MonitoringAreaDlg::monitoringAreaInfo()
{
    MonitoringAreaInfo monitoringAreaInfo;
    monitoringAreaInfo.areaName = ui->editMonitoringAreaName->text();

    return monitoringAreaInfo;
}

void MonitoringAreaDlg::slotOk()
{
    if(ui->editMonitoringAreaName->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_monitoring_area_name);
        ui->editMonitoringAreaName->setFocus();
        return;
    }

    for(int i = 0; i < m_monitoringAreaInfos.size(); i ++)
    {
        if(m_monitoringAreaInfos[i].areaName == ui->editMonitoringAreaName->text() && m_editIndex != i)
        {
            QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Duplicated_monitoring_area_name);
            ui->editMonitoringAreaName->setFocus();
            return;
        }
    }

    done(1);
}

void MonitoringAreaDlg::slotCancel()
{
    done(0);
}

void MonitoringAreaDlg::retranslateUI()
{
    ui->lblMonitoringAreaName->setText(StringTable::Str_Monitoring_Area_Name_);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);

    setWindowTitle(StringTable::Str_Monitoring_Area);

}
