#include "blackfaceadddlg.h"
#include "ui_blackfaceadddlg.h"
#include "nextbuttonitem.h"
#include "frameresultitem.h"
#include "stringtable.h"
#include "imageprocessingsocket.h"
#include "serverinfosocket.h"

#include <QtWidgets>

#define INIT_DETECTION_ITEM_COUNT 50

BlackFaceAddDlg::BlackFaceAddDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlackFaceAddDlg)
{
    ui->setupUi(this);
    retranslateUI();

    m_serverInfoSocket = NULL;
    m_imageProessingSocket = NULL;
    m_maximumCount = 1;

    m_toolbar = new QToolBar;
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbar->addAction(ui->actionAddImage);

    QVBoxLayout* toolbarLayout = new QVBoxLayout;
    toolbarLayout->addWidget(m_toolbar);
    toolbarLayout->setMargin(0);
    toolbarLayout->setSpacing(0);
    ui->widToolBar->setLayout(toolbarLayout);

    m_detectionScene = new QGraphicsScene;
    ui->viewDetection->setScene(m_detectionScene);
    ui->viewDetection->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->viewDetection->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->viewDetection->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_enrollScene = new QGraphicsScene;
    ui->viewEnroll->setScene(m_enrollScene);
    ui->viewEnroll->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->viewEnroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->viewEnroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(ui->actionAddImage, SIGNAL(triggered()), this, SLOT(slotAddImage()));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(ui->btnDown, SIGNAL(clicked()), this, SLOT(slotDown()));
    connect(ui->btnUp, SIGNAL(clicked()), this, SLOT(slotUp()));

    constructDetectionResultItems();
    constructEnrollResultItems();
}

BlackFaceAddDlg::~BlackFaceAddDlg()
{
    delete ui;
}

void BlackFaceAddDlg::setInfo(ServerInfoSocket* serverInfoSocket, int maximumCount)
{
    m_serverInfoSocket = serverInfoSocket;
    m_maximumCount = maximumCount;
}

QVector<FRAME_RESULT> BlackFaceAddDlg::enrollFrameResults()
{
    return m_enrollFrameResults;
}

void BlackFaceAddDlg::slotOk()
{
    done(1);
}

void BlackFaceAddDlg::slotCancel()
{
    done(0);
}

void BlackFaceAddDlg::slotDown()
{
    if(m_enrollFrameResults.size() >= m_maximumCount)
        return;

    int exist = -1;
    for(int i = 0; i < m_detectionFrameItems.size(); i ++)
    {
        if(m_detectionFrameItems[i]->isSelected() && i < m_detectionFrameResults.size())
        {
            exist = i;
            break;
        }
    }

    if(exist >= 0)
    {
        m_enrollFrameResults.append(m_detectionFrameResults[exist]);
        m_detectionFrameResults.remove(exist);

        for(int i = 0; i < m_detectionFrameItems.size(); i ++)
            m_detectionFrameItems[i]->setEmpty();

        refreshDetectionResultItems();
        refreshEnrollResultItems();
    }
}

void BlackFaceAddDlg::slotUp()
{
    int exist = -1;
    for(int i = 0; i < m_enrollFrameItems.size(); i ++)
    {
        if(m_enrollFrameItems[i]->isSelected() && i < m_enrollFrameResults.size())
        {
            exist = i;
            break;
        }
    }

    if(exist >= 0)
    {
        m_detectionFrameResults.append(m_enrollFrameResults[exist]);
        m_enrollFrameResults.remove(exist);

        for(int i = 0; i < m_enrollFrameItems.size(); i ++)
            m_enrollFrameItems[i]->setEmpty();

        refreshDetectionResultItems();
        refreshEnrollResultItems();
    }
}

void BlackFaceAddDlg::slotAddImage()
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString fileName = QFileDialog::getOpenFileName(this, StringTable::Str_Open_File, setting.value("last enroll file dir").toString(),
                                                    tr("Image Files(*.bmp *.jpg *.jpeg *.tif *.tiff)"));
    if(fileName.isEmpty())
        return;

    QFileInfo info(fileName);
    setting.setValue("last enroll file dir", info.absoluteDir().absolutePath());

    QImage fileImage(fileName);
    if(fileImage.isNull())
        return;

    setSelectedImage(fileImage);
}


void BlackFaceAddDlg::slotDetectionPrevClicked()
{
    int curVal = ui->viewDetection->horizontalScrollBar()->value();
    int nextCount = curVal / (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP);
    if(nextCount == 0)
        ui->viewDetection->horizontalScrollBar()->setValue(0);
    else
    {
        nextCount --;
        ui->viewDetection->horizontalScrollBar()->setValue((FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP) * nextCount);
    }
    m_detectionNextItem->setPos(ui->viewDetection->horizontalScrollBar()->value() + ui->viewDetection->width() - NEXT_BUTTON_ITEM_WIDTH, 0);
    m_detectionPrevItem->setPos(ui->viewDetection->horizontalScrollBar()->value(), 0);
}


void BlackFaceAddDlg::slotDetectionNextClicked()
{
    int maxVal = ui->viewDetection->horizontalScrollBar()->maximum();
    int curVal = ui->viewDetection->horizontalScrollBar()->value();
    int nextCount = (maxVal - curVal) / (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP);
    if(nextCount == 0)
        ui->viewDetection->horizontalScrollBar()->setValue(maxVal);
    else
    {
        nextCount --;
        ui->viewDetection->horizontalScrollBar()->setValue(maxVal - (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP) * nextCount - FRAME_RESULT_ITEM_GAP);
    }
    m_detectionNextItem->setPos(ui->viewDetection->horizontalScrollBar()->value() + ui->viewDetection->width() - NEXT_BUTTON_ITEM_WIDTH, 0);
    m_detectionPrevItem->setPos(ui->viewDetection->horizontalScrollBar()->value(), 0);
}


void BlackFaceAddDlg::slotEnrollPrevClicked()
{
    int curVal = ui->viewEnroll->horizontalScrollBar()->value();
    int nextCount = curVal / (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP);
    if(nextCount == 0)
        ui->viewEnroll->horizontalScrollBar()->setValue(0);
    else
    {
        nextCount --;
        ui->viewEnroll->horizontalScrollBar()->setValue((FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP) * nextCount);
    }
    m_enrollNextItem->setPos(ui->viewEnroll->horizontalScrollBar()->value() + ui->viewEnroll->width() - NEXT_BUTTON_ITEM_WIDTH, 0);
    m_enrollPrevItem->setPos(ui->viewEnroll->horizontalScrollBar()->value(), 0);
}


void BlackFaceAddDlg::slotEnrollNextClicked()
{
    int maxVal = ui->viewEnroll->horizontalScrollBar()->maximum();
    int curVal = ui->viewEnroll->horizontalScrollBar()->value();
    int nextCount = (maxVal - curVal) / (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP);
    if(nextCount == 0)
        ui->viewEnroll->horizontalScrollBar()->setValue(maxVal);
    else
    {
        nextCount --;
        ui->viewEnroll->horizontalScrollBar()->setValue(maxVal - (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP) * nextCount - FRAME_RESULT_ITEM_GAP);
    }
    m_enrollNextItem->setPos(ui->viewEnroll->horizontalScrollBar()->value() + ui->viewEnroll->width() - NEXT_BUTTON_ITEM_WIDTH, 0);
    m_enrollPrevItem->setPos(ui->viewEnroll->horizontalScrollBar()->value(), 0);
}


void BlackFaceAddDlg::constructDetectionResultItems()
{
    m_detectionScene->clear();
    m_detectionFrameItems.clear();

    for(int i = 0; i < INIT_DETECTION_ITEM_COUNT; i ++)
    {
        FrameResultItem* frameResultItem = new FrameResultItem;
        m_detectionScene->addItem(frameResultItem);
        m_detectionFrameItems.append(frameResultItem);
    }

    m_detectionPrevItem = new NextButtonItem;
    m_detectionPrevItem->setPixmap(QPixmap(":/images/prev.png"));
    m_detectionScene->addItem(m_detectionPrevItem);

    m_detectionNextItem = new NextButtonItem;
    m_detectionNextItem->setPixmap(QPixmap(":/images/next.png"));
    m_detectionScene->addItem(m_detectionNextItem);

    connect(m_detectionPrevItem, SIGNAL(clicked()), this, SLOT(slotDetectionPrevClicked()));
    connect(m_detectionNextItem, SIGNAL(clicked()), this, SLOT(slotDetectionNextClicked()));
}

void BlackFaceAddDlg::refreshDetectionResultItems()
{
    if(m_detectionFrameResults.size() > m_detectionFrameItems.size())
    {
        int addCount = m_detectionFrameResults.size() - m_detectionFrameItems.size();
        for(int i = 0; i < addCount; i ++)
        {
            FrameResultItem* frameResultItem = new FrameResultItem;
            m_detectionScene->addItem(frameResultItem);
            m_detectionFrameItems.append(frameResultItem);
        }
    }

    for(int i = 0; i < m_detectionFrameResults.size(); i ++)
        m_detectionFrameItems[i]->setInfo(m_detectionFrameResults[i]);

    relocateDetectionResultItems();
}

void BlackFaceAddDlg::relocateDetectionResultItems()
{
    QRect sceneRect(0, 0, NEXT_BUTTON_ITEM_WIDTH * 2 + m_detectionFrameItems.size() * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), FRAME_RESULT_ITEM_HEIGHT);
    m_detectionScene->setSceneRect(sceneRect);
    for(int i = 0; i < m_detectionFrameItems.size(); i ++)
        m_detectionFrameItems[i]->setPos(NEXT_BUTTON_ITEM_WIDTH + i * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), 0);

    m_detectionPrevItem->setPos(0, 0);
    m_detectionNextItem->setPos(ui->viewDetection->width() - NEXT_BUTTON_ITEM_WIDTH, 0);

    m_detectionScene->update();
}

void BlackFaceAddDlg::constructEnrollResultItems()
{
    m_enrollScene->clear();
    m_enrollFrameItems.clear();

    for(int i = 0; i < INIT_DETECTION_ITEM_COUNT; i ++)
    {
        FrameResultItem* frameResultItem = new FrameResultItem;
        m_enrollScene->addItem(frameResultItem);
        m_enrollFrameItems.append(frameResultItem);
    }

    m_enrollPrevItem = new NextButtonItem;
    m_enrollPrevItem->setPixmap(QPixmap(":/images/prev.png"));
    m_enrollScene->addItem(m_enrollPrevItem);

    m_enrollNextItem = new NextButtonItem;
    m_enrollNextItem->setPixmap(QPixmap(":/images/next.png"));
    m_enrollScene->addItem(m_enrollNextItem);

    connect(m_enrollPrevItem, SIGNAL(clicked()), this, SLOT(slotEnrollPrevClicked()));
    connect(m_enrollNextItem, SIGNAL(clicked()), this, SLOT(slotEnrollNextClicked()));
}

void BlackFaceAddDlg::refreshEnrollResultItems()
{
    if(m_enrollFrameResults.size() > m_enrollFrameItems.size())
    {
        for(int i = 0; i < m_enrollFrameResults.size() - m_enrollFrameItems.size(); i ++)
        {
            FrameResultItem* frameResultItem = new FrameResultItem;
            m_enrollScene->addItem(frameResultItem);
            m_enrollFrameItems.append(frameResultItem);
        }
    }

    for(int i = 0; i < m_enrollFrameResults.size(); i ++)
        m_enrollFrameItems[i]->setInfo(m_enrollFrameResults[i]);

    relocateEnrollResultItems();
}

void BlackFaceAddDlg::relocateEnrollResultItems()
{
    QRect sceneRect(0, 0, NEXT_BUTTON_ITEM_WIDTH * 2 + m_enrollFrameItems.size() * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), FRAME_RESULT_ITEM_HEIGHT);
    m_enrollScene->setSceneRect(sceneRect);
    for(int i = 0; i < m_enrollFrameItems.size(); i ++)
        m_enrollFrameItems[i]->setPos(NEXT_BUTTON_ITEM_WIDTH + i * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), 0);

    m_enrollPrevItem->setPos(0, 0);
    m_enrollNextItem->setPos(ui->viewEnroll->width() - NEXT_BUTTON_ITEM_WIDTH, 0);

    m_enrollScene->update();
}

void BlackFaceAddDlg::setSelectedImage(QImage sceneImage)
{
    m_sceneImage = sceneImage;
    updateSceneImage();

    startImageProcessing();
}

void BlackFaceAddDlg::updateSceneImage()
{
    if(!m_sceneImage.isNull())
    {
        QRect sceneImageRect = getFrameRect(ui->lblScene->rect(), m_sceneImage.size());
        QImage scaledImage = m_sceneImage.scaled(sceneImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->lblScene->setPixmap(QPixmap::fromImage(scaledImage));
    }
}

void BlackFaceAddDlg::resizeEvent(QResizeEvent* e)
{
    QDialog::resizeEvent(e);
    updateSceneImage();

    relocateDetectionResultItems();
    relocateEnrollResultItems();
}


void BlackFaceAddDlg::startImageProcessing()
{
    stopImageProcessing();

    if(m_serverInfoSocket == NULL || m_sceneImage.isNull())
        return;

    m_imageProessingSocket = new ImageProcessingSocket;
    connect(m_imageProessingSocket, SIGNAL(receiveFrameResults(QVector<FRAME_RESULT>)), this, SLOT(receiveFrameResults(QVector<FRAME_RESULT>)));
    connect(m_imageProessingSocket, SIGNAL(finished()), this, SLOT(stopImageProcessing()));

    m_imageProessingSocket->startProcessing(m_serverInfoSocket->serverInfo(), m_sceneImage);
}

void BlackFaceAddDlg::stopImageProcessing()
{
    if(m_imageProessingSocket)
    {
        m_imageProessingSocket->stopProcessing();
        delete m_imageProessingSocket;
        m_imageProessingSocket = NULL;
    }
}

void BlackFaceAddDlg::receiveFrameResults(QVector<FRAME_RESULT> frameResults)
{
    m_detectionFrameResults = frameResults;

    refreshDetectionResultItems();
}

void BlackFaceAddDlg::retranslateUI()
{
    ui->lblFaceDetection->setText(StringTable::Str_Face_Detection);
    ui->lblEnrolledFace->setText(StringTable::Str_Enrolled_Face);
    ui->btnDown->setText(StringTable::Str__Down);
    ui->btnUp->setText(StringTable::Str__Up);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);
    ui->actionAddImage->setText(StringTable::Str_Select_Image);

    setWindowTitle(StringTable::Str_Add_Face);

}
