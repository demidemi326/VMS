#ifndef NEWPASS_H
#define NEWPASS_H

#include <QDialog>

namespace Ui {
class NewPass;
}

class NewPass : public QDialog
{
    Q_OBJECT
    
public:
    explicit NewPass(QWidget *parent = 0);
    ~NewPass();

public slots:
    void    slotOk();
    void    slotCancel();
    
private:
    void retranslateUI();

    Ui::NewPass *ui;
};

#endif // NEWPASS_H
