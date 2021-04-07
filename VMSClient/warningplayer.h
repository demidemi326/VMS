#ifndef WARNINGPLAYER_H
#define WARNINGPLAYER_H

#include <QThread>
#include <QMutex>
#include <QSound>

class WarningPlayer : public QObject
{
    Q_OBJECT
public:
    explicit WarningPlayer(QObject *parent = 0);

    static void    play(int sound);

    void    playSound(int);

signals:

public slots:

private:
    QMutex  m_mutex;
    int     m_running;
    int     m_status;

    QSound*  m_warning;
    QSound*  m_safe;
};

#endif // WARNINGPLAYER_H
