#include "aboutdlg.h"
#include "ui_aboutdlg.h"
#include "stringtable.h"

AboutDlg::AboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDlg)
{
    ui->setupUi(this);

    retranslateUI();
}

AboutDlg::~AboutDlg()
{
    delete ui;
}

void AboutDlg::retranslateUI()
{
    setWindowTitle(StringTable::Str_About);
}
