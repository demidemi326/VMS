#ifndef ACTIVATION_DLG_H
#define ACTIVATION_DLG_H

#include <QDialog>

namespace Ui {
class ActivationDlg;
}

class ActivationDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ActivationDlg(QWidget *parent = 0);
    ~ActivationDlg();

public slots:
    void    slotOk();
    void    slotCancel();

private:
    Ui::ActivationDlg *ui;

    int     m_activated;
};

#endif // DIALOG_H
