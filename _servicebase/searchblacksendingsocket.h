#ifndef SEARCHBLACKSENDINGSOCKET_H
#define SEARCHBLACKSENDINGSOCKET_H

#include "chaneldatasendingsocket.h"

class SearchBlackSendingSocket : public ChanelDataSendingSocket
{
    Q_OBJECT
public:
    explicit SearchBlackSendingSocket(QObject *parent = 0);

protected:
    void    run();
};

#endif // SEARCHBLACKSENDINGSOCKET_H
