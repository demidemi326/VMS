#ifndef IMAGESEARCHENGINE_H
#define IMAGESEARCHENGINE_H

#include <QThread>
#include <QMutex>
#include <QStringList>

class ImageSearchEngine : public QThread
{
    Q_OBJECT
public:
    ImageSearchEngine();
    ~ImageSearchEngine();

    void    searchImages(QString rootPath);

    void    stopEngine();

signals:
    void    searchResults(QStringList);

protected:
    void     run();

    void    processFolder(QString folderPath);
    void    processFiles(QString folderPath);

private:
    QString m_rootPath;
    int     m_running;
    QMutex  m_mutex;

    QStringList             m_imageFilters;
    QStringList             m_imageFiles;
};

#endif // IMAGESEARCHENGINE_H
