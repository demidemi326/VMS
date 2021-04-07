#ifndef SEARCHLASTBLACKSENDINGSOCKET_H
#define SEARCHLASTBLACKSENDINGSOCKET_H

#include "chaneldatasendingsocket.h"

#include <QDate>
#include <QVector>

class SearchLastBlackSendingSocket : public ChanelDataSendingSocket
{
    Q_OBJECT
public:
    explicit SearchLastBlackSendingSocket(QObject *parent = 0);

    void setSavedLogDate(QVector<QDate> logSavedDate);
signals:

public slots:

protected:
    void    run();

    QVector<QDate>  m_logSavedDate;

};

#endif // SEARCHLASTBLACKSENDINGSOCKET_H
