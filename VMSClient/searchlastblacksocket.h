#ifndef SEARCHLASTBLACKSOCKET_H
#define SEARCHLASTBLACKSOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class SearchLastBlackSocket : public QThread
{
    Q_OBJECT
public:
    explicit SearchLastBlackSocket(QObject *parent = 0);

    void    startSocket(QVector<int> serverIndexs, QVector<int> chanelIndexs, QVector<ServerInfoSocket*> serverInfoSockets);
    void    stopSocket();
signals:
    void    socketFinished(QObject* obj);
    void    receivedLastBlackResults(QVector<BLACK_RECOG_RESULT>);

public slots:

protected:
    void    run();

    int     authenticateSocket();
    void    receiveBlackResults();
private:
    SOCKET      m_socket;
    int         m_running;
    QMutex      m_mutex;

    ServerInfo  m_serverInfo;
    int         m_chanelIndex;

    int         m_receivedBlackCount;

    QVector<int>    m_serverIndexs;
    QVector<int>    m_chanelIndexs;
    QVector<ServerInfoSocket*> m_serverInfoSockets;

    QVector<BLACK_RECOG_RESULT> m_blackRecogResult;
};

#endif // SEARCHLASTBLACKSOCKET_H
