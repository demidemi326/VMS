#ifndef SEARCHBLACKSOCKET_H
#define SEARCHBLACKSOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class ServerInfoSocket;
class SearchBlackSocket : public QThread
{
    Q_OBJECT
public:
    explicit SearchBlackSocket(QObject *parent = 0);

    void    startSocket(QVector<int> serverIndexs, QVector<int> chanelIndexs, QVector<QString> areaNames, QVector<ServerInfoSocket*> serverInfoSockets,
                        QDateTime startTime, QDateTime endTime, QString searchName);
    void    stopSocket();

signals:
    void    socketFinished(QObject* obj);
    void    receivedBlackRecogResult(QVector<BLACK_RECOG_RESULT>);

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
    QDateTime   m_startTime;
    QDateTime   m_endTime;
    QString     m_searchName;

    QString     m_areaName;

    int         m_receivedBlackCount;

    QVector<int>    m_serverIndexs;
    QVector<int>    m_chanelIndexs;
    QVector<QString> m_areaNames;
    QVector<ServerInfoSocket*> m_serverInfoSockets;
};

#endif // SEARCHBLACKSOCKET_H
