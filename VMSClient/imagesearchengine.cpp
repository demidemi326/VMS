#include "imagesearchengine.h"

#include <QtWidgets>

ImageSearchEngine::ImageSearchEngine()
{
    m_running = 0;

    m_imageFilters.append("bmp");
    m_imageFilters.append("jpg");
    m_imageFilters.append("jpeg");
    m_imageFilters.append("png");
    m_imageFilters.append("tif");
    m_imageFilters.append("tiff");
}

ImageSearchEngine::~ImageSearchEngine()
{
    stopEngine();
}

void ImageSearchEngine::searchImages(QString rootPath)
{
    stopEngine();

    m_rootPath = rootPath;
    m_imageFiles.clear();

    start();
}

void ImageSearchEngine::stopEngine()
{
    m_mutex.lock();
    m_running = 0;
    m_mutex.unlock();

    wait();
}

void ImageSearchEngine::run()
{
    m_running = 1;

    processFolder(m_rootPath);

    if(m_running)
        searchResults(m_imageFiles);

    m_running = 0;
}

void ImageSearchEngine::processFolder(QString folderPath)
{
    QDir currentDir(folderPath);
    QStringList entryList = currentDir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs);

    for(int i = 0; i < entryList.size(); i ++)
    {
        m_mutex.lock();
        int running = m_running;
        m_mutex.unlock();

        if(running == 0)
            break;

        processFolder(folderPath + "/" + entryList[i]);
    }

    processFiles(folderPath);
}

void ImageSearchEngine::processFiles(QString folderPath)
{
    QDir currentDir(folderPath);
    QFileInfoList entryList = currentDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

    for(int i = 0; i < entryList.count(); i ++)
    {
        QString ext = entryList[i].suffix().toLower();
        int isExist = -1;
        for(int j = 0; j < m_imageFilters.count(); j ++)
        {
            if(ext == m_imageFilters[j])
            {
                isExist = j;
                break;
            }
        }

        if(isExist >= 0)
            m_imageFiles.append(entryList[i].filePath());
    }
}
