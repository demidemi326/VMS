#ifndef BLACKRECOGRESULTRECEIVESOCKET_H
#define BLACKRECOGRESULTRECEIVESOCKET_H

#include "clientbase.h"

#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class ServerInfoSocket;
class BlackRecogResultReceiveSocket : public QThread
{
    Q_OBJECT
public:
    explicit BlackRecogResultReceiveSocket(QObject *parent = 0);

    void    setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex);
    void    stopSocket();

    ServerInfoSocket*   serverInfoSocket();
    int                 chanelIndex();
signals:
    void    receivedBlackRecogResult(QVector<BLACK_RECOG_RESULT>);

public slots:
    void    slotReconnect();
    void    slotChanelStatusChanged(int serverIndex, int chanelIndex, int chanelStatus);

protected:
    void    run();

    int     authenticateSocket();
    void    receiveBlackRecogResult();
private:
    SOCKET      m_socket;

    int                     m_chanelIndex;
    ServerInfoSocket*       m_serverInfoSocket;
};

#endif // BLACKRECOGRESULTRECEIVESOCKET_H
