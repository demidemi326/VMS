#ifndef PROCESSCLUSTERING_H
#define PROCESSCLUSTERING_H

#include <QObject>

class ProcessClustering : public QObject
{
    Q_OBJECT
public:
    explicit ProcessClustering(QObject *parent = 0);

    void    processClustering(QString rootPath);

signals:

public slots:

private:
    QStringList separateFrameFeatures(QStringList& allFeature);

private:
    QString m_rootPath;

};

#endif // PROCESSCLUSTERING_H
