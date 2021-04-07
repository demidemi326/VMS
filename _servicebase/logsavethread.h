#ifndef LOGSAVETHREAD_H
#define LOGSAVETHREAD_H

#include "servicebase.h"
#include <QThread>

class LogSaveThread : public QThread
{
    Q_OBJECT
public:
    explicit LogSaveThread(QObject *parent = 0);

    void    startSaveLog(QVector<QVector<BLACK_RECOG_INFO> > blackRecogResults, QVector<LOG_ITEM> logItems, int chanelIndex);

signals:
    void    saveFinished(QObject*);
    void    savedLogInfo(QDate savedLogDate);
public slots:

protected:
    void    run();

private:

    QVector<LOG_ITEM> m_logItems;
    int             m_chanelIndex;
    QVector<QVector<BLACK_RECOG_INFO> > m_blackRecogResults;
};

#endif // LOGSAVETHREAD_H
