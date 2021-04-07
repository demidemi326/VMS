#include "processclustering.h"
#include "cluster.h"

#include <QtWidgets>

ProcessClustering::ProcessClustering(QObject *parent) :
    QObject(parent)
{
}

void ProcessClustering::processClustering(QString rootPath)
{
//    m_rootPath = rootPath;

//    QString nameFilter;
//    nameFilter.sprintf("*.bin");

//    QString featureDirPath = rootPath + "/_feature/";
//    QDir featureDir(featureDirPath);
//    QStringList entryList = featureDir.entryList(QStringList(nameFilter), QDir::Files, QDir::Name);

//    QVector<Cluster*> clusters;
//    int index = 0, count = 0;
//    while(1)
//    {
//        QStringList frameFeatures = separateFrameFeatures(entryList);
//        if(frameFeatures.size() == 0)
//            break;

//        for(int i = 0; i < clusters.size(); i ++)
//        {
//            bool ret = clusters[i]->doClustering(frameFeatures);
//            if(!ret)
//                clusters[i]->increaseFinish();
//        }

//        for(int i = 0; i < frameFeatures.size(); i ++)
//        {
//            Cluster* newCluster = new Cluster;
//            newCluster->setFirstFeature(m_rootPath, frameFeatures[i]);

//            clusters.append(newCluster);
//        }

////        for(int i = 0; i < clusters.size(); i ++)
////        {
////            int exist = -1;
////            for(int j = i + 1; j < clusters.size(); j ++)
////            {
////                if(clusters[i]->canMerge(clusters[j]))
////                {
////                    exist = j;
////                    break;
////                }
////            }

////            if(exist >= 0)
////            {
////                clusters[i]->mergeCluster(clusters[exist]);
////                clusters.remove(exist);
////            }
////        }
//        if(count == 100)
//            break;

//        for(int i = 0; i < clusters.size(); i ++)
//        {
//            if(clusters[i]->isFinished())
//            {
//                clusters[i]->save(index);
//                delete clusters[i];
//                clusters.remove(i);
//                i --;

//                index ++;
//            }
//        }

//        count++;
//    }

//    for(int i = 0; i < clusters.size(); i ++)
//    {
//        clusters[i]->save(index + i);
//    }

//    qDebug() << "cluster " << clusters.size();
}

QStringList ProcessClustering::separateFrameFeatures(QStringList& allFeature)
{
    QStringList newFrameFeatures;
//    if(allFeature.size() == 0)
//        return newFrameFeatures;

//    QStringList splitList = allFeature[0].split("_");
//    if(splitList.size() != 3)
//        return newFrameFeatures;

//    QString prefixFeature = splitList[0];
//    for(int i = 0; i < allFeature.size(); i ++)
//    {
//        splitList = allFeature[i].split("_");
//        if(splitList.size() != 3)
//            continue;

//        if(prefixFeature == splitList[0])
//        {
//            newFrameFeatures.append(allFeature[i]);
//            allFeature.removeAt(0);
//            i --;
//        }
//        else
//            break;
//    }

    return newFrameFeatures;
}
