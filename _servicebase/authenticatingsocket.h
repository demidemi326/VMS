#ifndef AUTHENTICATINGSOCKET_H
#define AUTHENTICATINGSOCKET_H

#include <QtWidgets>
#include <winsock.h>


#define LISTEN_TYPE_SERVER_INFO 0
#define LISTEN_TYPE_FACE_RESULT 1
#define LISTEN_TYPE_FACE_IMAGE 2
#define LISTEN_TYPE_SCENE_IMAGE 3
#define LISTEN_TYPE_IMAGE_PROCESSING 4
#define LISTEN_TYPE_ONE_BLACK 5
#define LISTEN_TYPE_BLACK_INFO 6
#define LISTEN_TYPE_EDIT_ONE_BLACK 7
#define LISTEN_TYPE_DELETE_BLACK 8
#define LISTEN_TYPE_BLACK_RECOG_RESULT 9
#define LISTEN_TYPE_SEARCH_LOG 10
#define LISTEN_TYPE_SEARCH_BLACK 11
#define LISTEN_TYPE_SEARCH_LAST_BLACK 12

class AuthenticatingSocket : public QThread
{
    Q_OBJECT
public:
    explicit AuthenticatingSocket(QObject *parent = 0);

    void    startAuthenticating(SOCKET, SOCKADDR_IN, int);
    void    stopAuthenticate();

signals:
    void    authenticateFinished(QObject* obj);
    void    authenticatedSocket(int, SOCKET, SOCKADDR_IN, int);

public slots:

protected:
    void    run();
    int     authenticateSocket();

private:
    SOCKET          m_socket;
    SOCKADDR_IN     m_socketIn;
    int             m_socketInLen;
};

#endif // AUTHENTICATINGSOCKET_H
