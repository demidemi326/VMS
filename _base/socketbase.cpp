#include "socketbase.h"


int receiveStringBySocket(SOCKET socket, QString& receiveStr)
{
    receiveStr = "";
    char receivedData[256] = { 0 };
    int ret = recv(socket, receivedData, sizeof(receivedData), 0);
    if(ret <= 0)
        return 0;

    receiveStr = receivedData;
    return 1;
}

int receiveIntBySocket(SOCKET socket, int& recvNum)
{
    recvNum = 0;
    char receivedData[4] = { 0 };
    int ret = recv(socket, receivedData, sizeof(receivedData), 0);
    if(ret <= 0)
        return 0;

    recvNum = *(int*)receivedData;
    return 1;
}

int receiveDataBySocket(SOCKET socket, QByteArray& receiveData)
{
    int receiveDataSize;
    int ret = recv(socket, (char*)&receiveDataSize, sizeof(receiveDataSize), 0);
    if(ret <= 0 || receiveDataSize < 0)
        return 0;

    receiveData.resize(receiveDataSize);

    char* receiveDataPtr = receiveData.data();
    int receiveOff = 0;
    int recieveSize = PACKET_SIZE;

    while(1)
    {
        recieveSize = qMin(PACKET_SIZE, receiveData.size() -  receiveOff);
        ret = recv(socket, receiveDataPtr + receiveOff, recieveSize, 0);
        if(ret <= 0)
            return 0;

        receiveOff += ret;

        if(receiveOff >= receiveData.size())
            break;
    }

    return 1;
}


int sendStringBySocket(SOCKET socket, QString sendStr)
{
    char sendData[256] = { 0 };
    sprintf(sendData, sendStr.toUtf8().data());
    int ret = send(socket, sendData, sizeof(sendData), 0);
    if(ret <= 0)
        return 0;

    return 1;
}

int sendIntBySocket(SOCKET socket, int sendNum)
{
    char sendData[4] = { 0 };
    *(int*)sendData = sendNum;
    int ret = send(socket, sendData, sizeof(sendData), 0);
    if(ret <= 0)
        return 0;

    return 1;
}

int sendDataBySocket(SOCKET socket, QByteArray sendData)
{
    int sendDataSize = sendData.size();
    int ret = send(socket, (char*)&sendDataSize, sizeof(sendDataSize), 0);
    if(ret <= 0)
        return 0;

    char* sendDataPtr = sendData.data();
    int sendOff = 0;
    int sendSize = PACKET_SIZE;
    while(1)
    {
        sendSize = qMin(PACKET_SIZE, sendData.size() -  sendOff);
        ret = send(socket, sendDataPtr + sendOff, sendSize, 0);
        if(ret <= 0)
            return 0;

        sendOff += ret;

        if(sendOff >= sendData.size())
            break;
    }

    return 1;
}
