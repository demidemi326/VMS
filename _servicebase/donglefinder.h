#ifndef DONGLEFINDER_H
#define DONGLEFINDER_H

#include <QThread>
#include <QMutex>

class DongleFinder : public QThread
{
public:
    DongleFinder();
    ~DongleFinder();

    void    stopFind();

protected:
    void    run();

    int     m_running;
    QMutex  m_mutex;
};

#endif // DONGLEFINDER_H
