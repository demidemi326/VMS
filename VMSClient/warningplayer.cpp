#include "warningplayer.h"
#include <QtWidgets>
#include <QtMultimedia>

WarningPlayer* g_playerInstance = NULL;

WarningPlayer::WarningPlayer(QObject *parent) :
    QObject(parent)
{
    m_running = 0;
    m_warning = new QSound(":/sound/warning.wav");
    m_warning->setLoops(2);

    m_safe = new QSound(":/sound/safe.wav");
    m_safe->setLoops(2);
}

void WarningPlayer::play(int sound)
{
    if(g_playerInstance == NULL)
        g_playerInstance = new WarningPlayer;
    g_playerInstance->playSound(sound);

}

void WarningPlayer::playSound(int sound)
{
    if(m_warning->isFinished() && sound == 1)
        m_warning->play();
    else if(m_safe->isFinished() && sound == 0)
        m_safe->play();
}
