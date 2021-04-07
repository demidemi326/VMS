#ifndef DELETEBLACKLISTSOCKET_H
#define DELETEBLACKLISTSOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class DeleteBlackListSocket : public QThread
{
    Q_OBJECT
public:
    explicit DeleteBlackListSocket(QObject *parent = 0);

    void    setInfo(ServerInfo serverInfo, int chanelIndex, QStringList deleteNames);
    void    startDelete();
    void    stopDelete();

signals:
    void    deleteFinished(QObject*);

public slots:

protected:
    void    run();

    int     authenticateSocket();
    int     sendDeleteBlackPersonData();
private:
    SOCKET      m_socket;

    ServerInfo  m_serverInfo;
    int         m_chanelIndex;
    QStringList m_deleteNames;

};

#endif // DELETEBLACKLISTSOCKET_H
