#ifndef FACEREQUESTSOCKET_H
#define FACEREQUESTSOCKET_H

#include "clientbase.h"

#include <QThread>
#include <QMutex>

#include <winsock.h>
//#include <winsock2.h>

class ServerInfoSocket;
class FaceRectReceiveSocket : public QThread
{
    Q_OBJECT
public:
    explicit FaceRectReceiveSocket(QObject *parent = 0);

    void    setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex);
    void    startSocket(ServerInfo serverInfo, int chanelIndex);
    void    stopSocket();

signals:
    void    receivedFaceResult(QVector<QRect>, qint64, qint64, QImage);

public slots:
    void    slotReconnect();
    void    slotChanelStatusChanged(int serverIndex, int chanel, int status);

protected:
    void    run();

    void    recvFaceResult();
    int     authenticateSocket();
private:

    SOCKET      m_socket;

    int                     m_chanelIndex;
    ServerInfoSocket*       m_serverInfoSocket;
};

#endif // FACEREQUESTSOCKET_H
