#ifndef SCENEIMAGERECEIVESOCKET_H
#define SCENEIMAGERECEIVESOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class SceneImageReceiveSocket : public QThread
{
    Q_OBJECT
public:
    explicit SceneImageReceiveSocket(QObject *parent = 0);

    void    startSocket(ServerInfo serverInfo, QString logUID, int logType);
    void    stopSocket();

signals:
    void    receivedSceneData(FRAME_RESULT, QImage);

protected:
    void    run();

    int     authenticateSocket();
    void    recvSceneImage();
private:
    SOCKET      m_socket;

    ServerInfo              m_serverInfo;
    QString                 m_logUID;
    int                     m_logType;
};

#endif // SCENEIMAGERECEIVESOCKET_H
