#ifndef FACEIMAGERECEIVESOCKET_H
#define FACEIMAGERECEIVESOCKET_H

#include "clientbase.h"

#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class ServerInfoSocket;
class LogResultReceiveSocket : public QThread
{
    Q_OBJECT
public:
    explicit LogResultReceiveSocket(QObject *parent = 0);

    void    setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex);
    void    stopSocket();

    ServerInfoSocket*   serverInfoSocket();
    int                 chanelIndex();
signals:
    void    receivedLogResult(LOG_RESULT);

public slots:
    void    slotReconnect();
    void    slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus);

protected:
    void    run();

    int     authenticateSocket();
    void    recvFaceImage();
private:
    SOCKET      m_socket;

    int                     m_chanelIndex;
    ServerInfoSocket*       m_serverInfoSocket;
};

#endif // FACEIMAGERECEIVESOCKET_H
