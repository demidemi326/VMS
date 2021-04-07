#ifndef FACESENDINGSOCKET_H
#define FACESENDINGSOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class FaceSendingSocket : public QThread
{
    Q_OBJECT
public:
    explicit FaceSendingSocket(QObject *parent = 0);

    void    setSocketInfo(int, SOCKET, SOCKADDR_IN, int);
    void    stopSocket();

signals:
    void    socketError(QObject* obj);

public slots:
    void    slotSendFaceResults(QVector<QRect> faceResults, qint64 engineTime);

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;

    QVector<QRect>   m_faceResults;
    qint64         m_imageTime;
};

#endif // FACESENDINGSOCKET_H
