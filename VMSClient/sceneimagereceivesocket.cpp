#include "sceneimagereceivesocket.h"
#include "socketbase.h"

#include <QtWidgets>

SceneImageReceiveSocket::SceneImageReceiveSocket(QObject *parent) :
    QThread(parent)
{
}

void SceneImageReceiveSocket::startSocket(ServerInfo serverInfo, QString logUID, int logType)
{
    stopSocket();
    m_serverInfo = serverInfo;
    m_logUID = logUID;
    m_logType = logType;

    start();
}

void SceneImageReceiveSocket::stopSocket()
{
    closesocket(m_socket);

    wait();
}

void SceneImageReceiveSocket::run()
{
    SOCKADDR_IN target; //Information about host

    m_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (m_socket == INVALID_SOCKET)
        return;

    target.sin_family = AF_INET;           // address family Internet
    target.sin_port = htons (m_serverInfo.port);        // set server’s port number
    target.sin_addr.s_addr = inet_addr(m_serverInfo.ipAddress.toUtf8().data());  // set server’s IP

    if (::connect(m_socket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) //Try binding
        return;

    int ret = authenticateSocket();
    if(ret)
        recvSceneImage();

    closesocket(m_socket);
}

int SceneImageReceiveSocket::authenticateSocket()
{
    QString authenticatingStr;
    authenticatingStr = m_serverInfo.userName + "\n" + m_serverInfo.password;
    sendStringBySocket(m_socket, authenticatingStr);

    receiveStringBySocket(m_socket, authenticatingStr);
    if(authenticatingStr == "AUTH OK")
    {
        sendStringBySocket(m_socket, "SCENE_IMAGE");
        return 1;
    }
    else
        return 0;
}

void SceneImageReceiveSocket::recvSceneImage()
{
    sendStringBySocket(m_socket, m_logUID);
    sendIntBySocket(m_socket, m_logType);

    QByteArray receiveSceneData;
    int ret = receiveDataBySocket(m_socket, receiveSceneData);
    if(ret && receiveSceneData.size())
    {
        FRAME_RESULT frameResult;

        QByteArray sceneImageData;
        QBuffer receiveBuffer(&receiveSceneData);
        receiveBuffer.open(QIODevice::ReadOnly);

        QDataStream sceneDataStream(&receiveBuffer);
        sceneDataStream >> sceneImageData;
        sceneDataStream >> frameResult.featData;
        sceneDataStream >> frameResult.faceRect;
        receiveBuffer.close();


        QImage sceneImage = QImage::fromData(sceneImageData, "JPG");
        frameResult.faceImage = cropFaceImage(sceneImage, frameResult.faceRect);

        emit receivedSceneData(frameResult, sceneImage);
    }
}
