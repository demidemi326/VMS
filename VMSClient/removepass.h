#ifndef REMOVEPASS_H
#define REMOVEPASS_H

#include <QDialog>

namespace Ui {
class RemovePass;
}

class RemovePass : public QDialog
{
    Q_OBJECT
    
public:
    explicit RemovePass(QWidget *parent = 0);
    ~RemovePass();

public slots:
    void    slotOk();
    void    slotCancel();


private:
    void    retranslateUI();

private:
    Ui::RemovePass *ui;
};

#endif // REMOVEPASS_H
