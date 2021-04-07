#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>


namespace Ui {
class LoginDlg;
}
class QEvent;
class LoginDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoginDlg(QWidget *parent = 0);
    ~LoginDlg();

public slots:
    void        slotOk();
    void        slotCancel();

private:
    void        retranslateUI();

private:
        Ui::LoginDlg *ui;
};

#endif // LOGINDLG_H
