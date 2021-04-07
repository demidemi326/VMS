#include "authenticatingsocket.h"
#include "servicebase.h"
#include "socketbase.h"

#include <QtWidgets>

AuthenticatingSocket::AuthenticatingSocket(QObject *parent) :
    QThread(parent)
{
}

void AuthenticatingSocket::startAuthenticating(SOCKET socket, SOCKADDR_IN socketIn, int socketInLen)
{
    m_socket = socket;
    m_socketIn = socketIn;
    m_socketInLen = socketInLen;

    start();
}

void AuthenticatingSocket::stopAuthenticate()
{
    closesocket(m_socket);

    wait();
}

void AuthenticatingSocket::run()
{
    int ret = authenticateSocket();
    if(ret)
    {
        QString receiveData;
        receiveStringBySocket(m_socket, receiveData);

        if(receiveData == "SERVER_INFO")
            emit authenticatedSocket(LISTEN_TYPE_SERVER_INFO, m_socket, m_socketIn, m_socketInLen);
        else if(receiveData == "FACE_RESULT")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_FACE_RESULT, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "FACE_IMAGE")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_FACE_IMAGE, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "SCENE_IMAGE")
            emit authenticatedSocket(LISTEN_TYPE_SCENE_IMAGE, m_socket, m_socketIn, m_socketInLen);
        else if(receiveData == "IMAGE_PROCESS")
            emit authenticatedSocket(LISTEN_TYPE_IMAGE_PROCESSING, m_socket, m_socketIn, m_socketInLen);
        else if(receiveData == "ONE_BLACK")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_ONE_BLACK, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "BLACK_INFO_RECEIVE")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_BLACK_INFO, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "EDIT_ONE_BLACK")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_EDIT_ONE_BLACK, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "DELETE_BLACK_INFO")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_DELETE_BLACK, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "BLACK_RECOG_RESULT")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_BLACK_RECOG_RESULT, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "SEARCH LOG")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_SEARCH_LOG, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "SEARCH BLACK")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_SEARCH_BLACK, m_socket, m_socketIn, chanelIndex);
        }
        else if(receiveData == "SEARCH LAST BLACK")
        {
            int chanelIndex;
            receiveIntBySocket(m_socket, chanelIndex);
            emit authenticatedSocket(LISTEN_TYPE_SEARCH_LAST_BLACK, m_socket, m_socketIn, chanelIndex);
        }
        else
            closesocket(m_socket);
    }
    else
        closesocket(m_socket);

    emit authenticateFinished(this);
}

int AuthenticatingSocket::authenticateSocket()
{
    QString authenticatingStr;
    receiveStringBySocket(m_socket, authenticatingStr);

    QStringList autehList = authenticatingStr.split("\n");
    if(autehList.size() == 2 && autehList[0] == getUserName() && autehList[1] == getPassword())
    {
        sendStringBySocket(m_socket, "AUTH OK");
        return 1;
    }
    else
    {
        sendStringBySocket(m_socket, "AUTH ERROR");
        return 0;
    }
}
