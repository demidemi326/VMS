#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cameraprocessengine.h"
#include "clientlisteningsocket.h"
#include "facesendingsocket.h"
#include "authenticatingsocket.h"
#include "faceimagesendingsocket.h"
#include "socketbase.h"
#include "serverinfosocket.h"

#include "surveillanceservice.h"
#include "frengine.h"
#include "processclustering.h"
#include "cluster.h"

#include <QtWidgets>

unsigned char yuvData[(int)(1920 * 1080 * 1.5)];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_gallery = NULL;
    m_probe = NULL;

    setupActions();


    m_clustering = new ProcessClustering;

    m_surveillanceService = new SurveillanceService;
    connect(m_surveillanceService, SIGNAL(logOut(QString)), this, SLOT(slotLogOut(QString)));

    m_surveillanceService->create();
}

MainWindow::~MainWindow()
{
    m_surveillanceService->release();

    delete ui;
}

void MainWindow::setupActions()
{
    connect(ui->btnStartService, SIGNAL(clicked()), this, SLOT(slotStartService()));
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(slotStop()));
//    connect(ui->btnClustering, SIGNAL(clicked()), this, SLOT(slotClustering()));

//    connect(ui->btnGallery, SIGNAL(clicked()), this, SLOT(slotGallery()));
//    connect(ui->btnProbe, SIGNAL(clicked()), this, SLOT(slotProbe()));
//    connect(ui->btnEval, SIGNAL(clicked()), this, SLOT(slotEval()));
}


void MainWindow::slotStartService()
{
    m_surveillanceService->start();
}

void MainWindow::slotStop()
{
    m_surveillanceService->stop();
}


void MainWindow::slotLogOut(QString logStr)
{
    logStr = logStr + "\n" + ui->textEdit->toPlainText();
    if(logStr.length() > 1000)
        logStr.resize(1000);
    ui->textEdit->setText(logStr);
}

void MainWindow::slotClustering()
{
    m_clustering->processClustering("D:/Temp/chanel 0");
}

void MainWindow::slotGallery()
{
//    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
//    QString fileName = QFileDialog::getOpenFileName(this, "", setting.value("last gallery file dir").toString(),
//                                                    tr(""));
//    if(fileName.isEmpty())
//        return;

//    QFileInfo info(fileName);
//    setting.setValue("last gallery file dir", info.absoluteDir().absolutePath());

//    if(m_gallery)
//        delete m_gallery;

//    m_gallery = new Cluster;
//    m_gallery->setFirstFeature("D:/Temp/chanel 0", info.fileName());

//    QString rectStr;
//    rectStr.sprintf("%d, %d, %d, %d", m_gallery->lastFileRect().left(), m_gallery->lastFileRect().top(), m_gallery->lastFileRect().right(), m_gallery->lastFileRect().bottom());
//    ui->editGallery->setText(rectStr + ", " + QString::number(m_gallery->lastFileTime()));

}

void MainWindow::slotProbe()
{
//    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
//    QString fileName = QFileDialog::getOpenFileName(this, "", setting.value("last gallery file dir").toString(),
//                                                    tr(""));
//    if(fileName.isEmpty())
//        return;

//    QFileInfo info(fileName);
//    setting.setValue("last gallery file dir", info.absoluteDir().absolutePath());

//    if(m_probe)
//        delete m_probe;

//    m_probe = new Cluster;
//    m_probe->setFirstFeature("D:/Temp/chanel 0", info.fileName());

//    QString rectStr;
//    rectStr.sprintf("%d, %d, %d, %d", m_probe->firstFileRect().left(), m_probe->firstFileRect().top(), m_probe->firstFileRect().right(), m_probe->firstFileRect().bottom());
//    ui->editProbe->setText(rectStr + ", " + QString::number(m_probe->firstFileTime()));

}

void MainWindow::slotEval()
{
//    ui->editEval->setText("");
//    if(m_gallery == NULL || m_probe == NULL)
//        return;

//    float rScore = m_gallery->getScore(m_probe);
//    bool intersects = m_gallery->lastFileRect().intersects(m_probe->firstFileRect());
//    qint64 firstTimeVal = m_probe->firstFileTime();
//    qint64 lastTimeVal = m_gallery->lastFileTime();
//    qint64 timeDiff = firstTimeVal - lastTimeVal;

//    QRect intersect = m_gallery->lastFileRect().intersected(m_probe->firstFileRect());
//    float clusterScore = rScore * rScore * (intersect.width()* intersect.height()) / (float)(m_gallery->lastFileRect().width() * m_gallery->lastFileRect().height() +
//                                                                                    m_probe->firstFileRect().width() * m_probe->firstFileRect().height());

//    QString evalStr;
//    evalStr.sprintf("Score = %f\nIntersects = %d\nTimeDiff = %d\nClusterScore = %f\r\n", rScore, intersects, (int)timeDiff, clusterScore);


//    ui->editEval->setText(evalStr);
}
