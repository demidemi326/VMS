#ifndef IMAGEPROCESSINGSOCKET_H
#define IMAGEPROCESSINGSOCKET_H

#include "servicebase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class ImageProcessingSocket : public QThread
{
    Q_OBJECT
public:
    explicit ImageProcessingSocket(QObject *parent = 0);

    void setSocketInfo(int chanelIndex, SOCKET socket, SOCKADDR_IN socketIn, int socketInLen);
    void stopSocket();

signals:
    void    processingFinshed(QObject*);

public slots:

protected:
    void    run();
    int     receiveImage();
    int     procssingImage();
    int     sendResults();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;

    QImage          m_image;
    int             m_chanelIndex;

    QVector<SENDING_FRAME_RESULT>   m_frameResults;
};

#endif // IMAGEPROCESSINGSOCKET_H
