#ifndef BLACKRECOGRESULTSENDINGSOCKET_H
#define BLACKRECOGRESULTSENDINGSOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class BlackRecogResultSendingSocket : public QThread
{
    Q_OBJECT
public:
    explicit BlackRecogResultSendingSocket(QObject *parent = 0);

    void    setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void    stopSocket();

signals:
    void    socketError(QObject* obj);

public slots:
    void    slotSendBlackRecogReuslts(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults,QVector<LOG_ITEM>,int);

protected:
    void    run();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
    int             m_chanelIndex;

    QVector<QVector<BLACK_RECOG_INFO> >   m_blackRecogResults;
};

#endif // BLACKRECOGRESULTSENDINGSOCKET_H
