#ifndef SCENEIMAGESENDSOCKET_H
#define SCENEIMAGESENDSOCKET_H

#include "servicebase.h"

#include <QThread>
#include <winsock.h>
//#include <winsock2.h>

class SceneImageSendSocket : public QThread
{
    Q_OBJECT
public:
    explicit SceneImageSendSocket(QObject *parent = 0);

    void setSocketInfo(SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void stopSocket();

signals:
    void    sendFinshed(QObject* );

public slots:

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
};

#endif // SCENEIMAGESENDSOCKET_H
