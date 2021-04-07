#ifndef SEARCHLOGSENDINGSOCKET_H
#define SEARCHLOGSENDINGSOCKET_H

#include "chaneldatasendingsocket.h"

class SearchLogSendingSocket : public ChanelDataSendingSocket
{
    Q_OBJECT
public:
    explicit SearchLogSendingSocket(QObject *parent = 0);

protected:
    void    run();
};

#endif // SEARCHLOGSENDINGSOCKET_H
