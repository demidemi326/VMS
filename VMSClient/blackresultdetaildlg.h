#ifndef BLACKRESULTDETAILDLG_H
#define BLACKRESULTDETAILDLG_H

#include <QDialog>

namespace Ui {
class BlackResultDetailDlg;
}

class BlackResultDetailDlg : public QDialog
{
    Q_OBJECT

public:
    explicit BlackResultDetailDlg(QWidget *parent = 0);
    ~BlackResultDetailDlg();

private:
    Ui::BlackResultDetailDlg *ui;
};

#endif // BLACKRESULTDETAILDLG_H
