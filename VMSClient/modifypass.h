#ifndef MODIFYPASS_H
#define MODIFYPASS_H

#include <QDialog>

namespace Ui {
class ModifyPass;
}

class ModifyPass : public QDialog
{
    Q_OBJECT
    
public:
    explicit ModifyPass(QWidget *parent = 0);
    ~ModifyPass();
    
public slots:
    void    slotOk();
    void    slotCancel();

private:
    void    retranslateUI();

private:
    Ui::ModifyPass *ui;
};

#endif // MODIFYPASS_H
