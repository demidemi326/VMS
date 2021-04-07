#include "donglefinder.h"
#include "frengine.h"

#include <QtWidgets>

DongleFinder::DongleFinder()
{
    m_running = 0;
}

DongleFinder::~DongleFinder()
{

}

void DongleFinder::stopFind()
{
    m_running = 0;
    wait();
}

void DongleFinder::run()
{
    m_mutex.lock();
    m_running = 1;
    m_mutex.unlock();
    while(m_running)
    {
        int faceCount = 0;
        FaceDetection(NULL, NULL, &faceCount);
        if(faceCount < 0)
        {
            qApp->quit();
        }

        QThread::msleep(500);
    }
}
