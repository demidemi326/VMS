#ifndef SOCKETBASE_H
#define SOCKETBASE_H

#include <QtWidgets>
#include <winsock.h>
//#include <winsock2.h>

#define VMS_SERVER_PORT 12101
#define PACKET_SIZE 1024


int receiveStringBySocket(SOCKET socket, QString& receiveStr);
int receiveIntBySocket(SOCKET socket, int& recvNum);
int receiveDataBySocket(SOCKET socket, QByteArray& receiveData);

int sendStringBySocket(SOCKET socket, QString sendStr);
int sendIntBySocket(SOCKET socket, int sendNum);
int sendDataBySocket(SOCKET socket, QByteArray sendData);


#endif // SOCKETBASE_H
