#ifndef EDITONEBLACKLISTSOCKET_H
#define EDITONEBLACKLISTSOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class EditOneBlackListSocket : public QThread
{
    Q_OBJECT
public:
    explicit EditOneBlackListSocket(QObject *parent = 0);

    void    setInfo(ServerInfo serverInfo, int chanelIndex, BLACK_PERSON blackPerson, QString oldName);
    void    startSocket();
    void    stopSocket();

signals:
    void    editFinished(QObject*);

public slots:

protected:
    void    run();

    int     authenticateSocket();
    int     sendBlackPersonData();

private:
    SOCKET      m_socket;

    ServerInfo  m_serverInfo;
    int         m_chanelIndex;

    BLACK_PERSON    m_blackPerson;
    QString     m_oldName;
};

#endif // EDITONEBLACKLISTSOCKET_H
