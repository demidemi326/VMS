#ifndef IMAGEPROCESSINGSOCKET_H
#define IMAGEPROCESSINGSOCKET_H

#include "clientbase.h"
#include <QThread>

#include <winsock.h>
//#include <winsock2.h>

class ImageProcessingSocket : public QThread
{
    Q_OBJECT
public:
    explicit ImageProcessingSocket(QObject *parent = 0);

    void    startProcessing(ServerInfo serverInfo, QImage image);
    void    stopProcessing();

signals:
    void    receiveFrameResults(QVector<FRAME_RESULT>);

public slots:

protected:
    void    run();

    int     authenticateSocket();
    int     sendProcessingImage();
    int     recvProcessingResult();

private:
    SOCKET      m_socket;

    ServerInfo  m_serverInfo;
    QImage      m_image;
};

#endif // IMAGEPROCESSINGSOCKET_H
