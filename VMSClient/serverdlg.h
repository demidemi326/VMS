#ifndef SERVERDLG_H
#define SERVERDLG_H

#include <QDialog>

#include "clientbase.h"

namespace Ui {
class ServerDlg;
}

class ServerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ServerDlg(QWidget *parent = 0);
    ~ServerDlg();

    void setServerInfo(QVector<ServerInfoSocket*> serverInfoSockets);
    void setEditServerInfo(QVector<ServerInfoSocket*> serverInfos, int index);
    ServerInfo serverInfo();

public slots:
    void    slotOk();
    void    slotCancel();

private:
    void    retranslateUI();

private:
    Ui::ServerDlg *ui;

    QVector<ServerInfoSocket*>     m_serverInfoSockets;
    int                     m_editIndex;
};

#endif // SERVERDLG_H
