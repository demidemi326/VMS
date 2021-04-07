#ifndef SEARCHLOGSOCKET_H
#define SEARCHLOGSOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class ServerInfoSocket;
class SearchLogSocket : public QThread
{
    Q_OBJECT
public:
    explicit SearchLogSocket(QObject *parent = 0);

    void    startSocket(QVector<int> serverIndexs, QVector<int> chanelIndexs, QVector<QString> areaNames, QVector<ServerInfoSocket*> serverInfoSockets,
                        QDateTime startTime, QDateTime endTime, QByteArray featData, int threshold);
    void    stopSocket();

signals:
    void    socketFinished(QObject* obj);
    void    receivedLogResult(LOG_RESULT);

public slots:

protected:
    void    run();

    int     authenticateSocket();
    void    receiveLogs();
private:
    SOCKET      m_socket;
    int         m_running;
    QMutex      m_mutex;

    QDateTime   m_startTime;
    QDateTime   m_endTime;
    QByteArray  m_featData;
    int         m_threshold;

    ServerInfo  m_serverInfo;
    int         m_chanelIndex;
    QString     m_areaName;

    int         m_receivedLogCount;

    QVector<int>    m_serverIndexs;
    QVector<int>    m_chanelIndexs;
    QVector<QString> m_areaNames;
    QVector<ServerInfoSocket*> m_serverInfoSockets;

};

#endif // SEARCHLOGSOCKET_H
