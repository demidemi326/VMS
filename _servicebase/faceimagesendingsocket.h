#ifndef FACEIMAGESENDINGSOCKET_H
#define FACEIMAGESENDINGSOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class FaceImageSendingSocket : public QThread
{
    Q_OBJECT
public:
    explicit FaceImageSendingSocket(QObject *parent = 0);

    void    setSocketInfo(int, SOCKET, SOCKADDR_IN, int);
    void    stopSocket();

signals:
    void    socketError(QObject* obj);

public slots:
    void    slotSendFaceImages(QVector<QVector<BLACK_RECOG_INFO> >, QVector<LOG_ITEM> logItems, int);

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;

    QVector<LOG_ITEM>   m_logItem;
};

#endif // FACEIMAGESENDINGSOCKET_H
