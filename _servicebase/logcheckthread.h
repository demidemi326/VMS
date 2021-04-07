#ifndef LOGCHECKTHREAD_H
#define LOGCHECKTHREAD_H

#include <QThread>
#include <QMutex>

#include "servicebase.h"

class LogCheckThread : public QThread
{
    Q_OBJECT
public:
    explicit LogCheckThread(QObject *parent = 0);

    void    startThread(QString logInfoName);
    void    stopThread();

    QVector<QDate>  logSavedDate();

signals:

public slots:

protected:
    void    run();
    void    removeAllDirContents(QString dirNameStr);

private:
    int     m_running;
    QMutex  m_mutex;

    QMutex                  m_logMutex;
    QVector<QDate>          m_logSavedDate;
    QVector<QDate>          m_tmpLogSavedDate;


    QString m_logItemInfoName;
};

#endif // LOGCHECKTHREAD_H
