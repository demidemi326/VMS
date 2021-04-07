#include "addblacklistfromcaptureddlg.h"
#include "ui_addblacklistfromcaptureddlg.h"
#include "stringtable.h"
#include "serverinfosocket.h"
#include "imageprocessingsocket.h"
#include "frameresultitem.h"
#include "nextbuttonitem.h"
#include "addblackinfosocket.h"
#include "frameresultitem.h"

#define INIT_DETECTION_ITEM_COUNT 50

AddBlackListFromCapturedDlg::AddBlackListFromCapturedDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddBlackListFromCapturedDlg)
{
    ui->setupUi(this);

    m_selectedAreaIndex = -1;
    m_selectedServerIndex = -1;
    m_selectedChanelIndex = -1;
    m_imageProessingSocket = NULL;

    setupActions();
    retranslateUI();
    constructDetectionResultItems();
    constructEnrollResultItems();
}

AddBlackListFromCapturedDlg::~AddBlackListFromCapturedDlg()
{
    stopImageProcessing();
    delete ui;
}

void AddBlackListFromCapturedDlg::setInfo(QVector<MonitoringAreaInfo> monitoringAreaInfos, QVector<ServerInfoSocket*> serverInfoSockets,
                                          int selectServerIndex, int selectChanelIndex, QImage sceneImage, int processEngine)
{
    m_monitoringAreaInfos = monitoringAreaInfos;
    m_serverInfoSockets = serverInfoSockets;
    m_sceneImage = sceneImage;

    m_selectedServerIndex = selectServerIndex;//selectedServerIndex(serverInfoSockets, faceImageInfo);
    m_selectedAreaIndex = getAreaIndexFromServerIndex(monitoringAreaInfos, m_selectedServerIndex, selectChanelIndex);
    m_selectedChanelIndex = selectChanelIndex;

    refreshAreaInfo();

    if(processEngine)
        startImageProcessing();
}

void AddBlackListFromCapturedDlg::addFrameResult(FRAME_RESULT frameResult)
{
    m_detectionFrameResults.append(frameResult);

    refreshDetectionResultItems();
}

void AddBlackListFromCapturedDlg::setupActions()
{
    m_monitoringAreaModel = new QStandardItemModel(0, 1);
    m_monitoringAreaModel->setHeaderData(0, Qt::Horizontal, StringTable::Str_Monitoring_Area);

    m_monitoringAreaSelectionModel = new QItemSelectionModel(m_monitoringAreaModel);

    ui->viewArea->setModel(m_monitoringAreaModel);
    ui->viewArea->setSelectionModel(m_monitoringAreaSelectionModel);
    ui->viewArea->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewArea->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->viewArea->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewArea->setAlternatingRowColors(true);

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

    connect(m_monitoringAreaModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(slotAreaItemDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(ui->btnDown, SIGNAL(clicked()), this, SLOT(slotDownClicked()));
    connect(ui->btnUp, SIGNAL(clicked()), this, SLOT(slotUpClicked()));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
}

void AddBlackListFromCapturedDlg::refreshAreaInfo()
{
    m_monitoringAreaModel->removeRows(0, m_monitoringAreaModel->rowCount());

    for(int i = 0; i < m_monitoringAreaInfos.size(); i ++)
    {
        MonitoringAreaInfo monitoringAreaInfo = m_monitoringAreaInfos[i];
        QStandardItem* monitoringAreaInfoItem = new QStandardItem;
        monitoringAreaInfoItem->setText(monitoringAreaInfo.areaName);
        monitoringAreaInfoItem->setIcon(QIcon(QPixmap(":/images/monitoring_area.png")));

        for(int j = 0; j < monitoringAreaInfo.serverIndexs.size(); j ++)
        {
            int selectedServerIndex = monitoringAreaInfo.serverIndexs[j];
            int selectedChanelIndex = monitoringAreaInfo.chanelIndexs[j];

            ServerInfo serverInfo = m_serverInfoSockets[selectedServerIndex]->serverInfo();

            QString serverInfoName = serverInfo.serverName;
            QString cameraItemText = serverInfoName + " (" + StringTable::Str_Chanel + QString::number(selectedChanelIndex + 1) + ")";
            QStandardItem* cameraItem = new QStandardItem;
            cameraItem->setText(cameraItemText);
            cameraItem->setIcon(QIcon(QPixmap(":/images/ipcamera.png")));
            cameraItem->setCheckable(true);

            monitoringAreaInfoItem->appendRow(cameraItem);
            monitoringAreaInfoItem->setCheckable(true);

            if(m_selectedAreaIndex == i)
            {
                cameraItem->setCheckState(Qt::Checked);
                monitoringAreaInfoItem->setCheckState(Qt::Checked);
            }
        }

        m_monitoringAreaModel->appendRow(monitoringAreaInfoItem);
        ui->viewArea->setExpanded(m_monitoringAreaModel->index(i, 0), true);
    }
}

void AddBlackListFromCapturedDlg::updateSceneImage()
{
    QRect sceneImageRect = getFrameRect(ui->lblScene->rect(), m_sceneImage.size());
    QImage scaledImage = m_sceneImage.scaled(sceneImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lblScene->setPixmap(QPixmap::fromImage(scaledImage));
}

void AddBlackListFromCapturedDlg::resizeEvent(QResizeEvent* e)
{
    QDialog::resizeEvent(e);
    e->accept();

    updateSceneImage();

    relocateDetectionResultItems();
    relocateEnrollResultItems();
}

void AddBlackListFromCapturedDlg::paintEvent(QPaintEvent *)
{
    updateSceneImage();
}


void AddBlackListFromCapturedDlg::startImageProcessing()
{
    stopImageProcessing();

    if(m_selectedServerIndex < 0 || m_selectedServerIndex >= m_serverInfoSockets.size() || m_sceneImage.isNull())
        return;

    m_imageProessingSocket = new ImageProcessingSocket;
    connect(m_imageProessingSocket, SIGNAL(receiveFrameResults(QVector<FRAME_RESULT>)), this, SLOT(slotReceiveFrameResults(QVector<FRAME_RESULT>)));
    connect(m_imageProessingSocket, SIGNAL(finished()), this, SLOT(stopImageProcessing()));

    m_imageProessingSocket->startProcessing(m_serverInfoSockets[m_selectedServerIndex]->serverInfo(), m_sceneImage);
}

void AddBlackListFromCapturedDlg::stopImageProcessing()
{
    if(m_imageProessingSocket)
    {
        m_imageProessingSocket->stopProcessing();
        delete m_imageProessingSocket;
        m_imageProessingSocket = NULL;
    }
}

void AddBlackListFromCapturedDlg::constructDetectionResultItems()
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

void AddBlackListFromCapturedDlg::refreshDetectionResultItems()
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

void AddBlackListFromCapturedDlg::relocateDetectionResultItems()
{
    QRect sceneRect(0, 0, NEXT_BUTTON_ITEM_WIDTH * 2 + m_detectionFrameItems.size() * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), FRAME_RESULT_ITEM_HEIGHT);
    m_detectionScene->setSceneRect(sceneRect);

    for(int i = 0; i < m_detectionFrameItems.size(); i ++)
        m_detectionFrameItems[i]->setPos(NEXT_BUTTON_ITEM_WIDTH + i * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), 0);

    m_detectionPrevItem->setPos(0, 0);
    m_detectionNextItem->setPos(ui->viewDetection->width() - NEXT_BUTTON_ITEM_WIDTH, 0);

    m_detectionScene->update();
}

void AddBlackListFromCapturedDlg::constructEnrollResultItems()
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

void AddBlackListFromCapturedDlg::refreshEnrollResultItems()
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

void AddBlackListFromCapturedDlg::relocateEnrollResultItems()
{
    QRect sceneRect(0, 0, NEXT_BUTTON_ITEM_WIDTH * 2 + m_enrollFrameItems.size() * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), FRAME_RESULT_ITEM_HEIGHT);
    m_enrollScene->setSceneRect(sceneRect);
    for(int i = 0; i < m_enrollFrameItems.size(); i ++)
        m_enrollFrameItems[i]->setPos(NEXT_BUTTON_ITEM_WIDTH + i * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), 0);

    m_enrollPrevItem->setPos(0, 0);
    m_enrollNextItem->setPos(ui->viewEnroll->width() - NEXT_BUTTON_ITEM_WIDTH, 0);

    m_enrollScene->update();
}


void AddBlackListFromCapturedDlg::slotAreaItemDataChanged(QModelIndex topLeft, QModelIndex, QVector<int>)
{
    QStandardItem* standardItem = m_monitoringAreaModel->itemFromIndex(topLeft);
    if(standardItem == NULL)
        return;

    if(standardItem->hasChildren())
    {
        for(int i = 0; i < standardItem->rowCount(); i ++)
            standardItem->child(i)->setCheckState(standardItem->checkState());
    }
    else if(standardItem->parent())
    {
        if(standardItem->checkState() == Qt::Unchecked)
        {
            int isChecked = 0;
            QStandardItem* parentItem = standardItem->parent();
            for(int i = 0; i < parentItem->rowCount(); i ++)
            {
                if(parentItem->child(i)->checkState() != Qt::Unchecked)
                    isChecked = 1;
            }

            if(isChecked == 0)
                parentItem->setCheckState(Qt::Unchecked);
        }
    }
}

void AddBlackListFromCapturedDlg::slotReceiveFrameResults(QVector<FRAME_RESULT> frameResults)
{
    m_detectionFrameResults = frameResults;

    refreshDetectionResultItems();
}

void AddBlackListFromCapturedDlg::slotDownClicked()
{
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

void AddBlackListFromCapturedDlg::slotUpClicked()
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

void AddBlackListFromCapturedDlg::slotDetectionPrevClicked()
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


void AddBlackListFromCapturedDlg::slotDetectionNextClicked()
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


void AddBlackListFromCapturedDlg::slotEnrollPrevClicked()
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


void AddBlackListFromCapturedDlg::slotEnrollNextClicked()
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

void AddBlackListFromCapturedDlg::slotOk()
{
    if(m_enrollFrameResults.size() == 0)
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_add_a_face);
        return;
    }

    if(ui->editName->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_a_name);
        ui->editName->setFocus();
        return;
    }

    QVector<BLACK_PERSON> blackInfos;
    BLACK_PERSON blackPerson;
    for(int i = 0; i < m_enrollFrameResults.size(); i ++)
        blackPerson.featData.append(m_enrollFrameResults[i].featData);

    blackPerson.name = ui->editName->text();
    blackPerson.gender = ui->comboGender->currentIndex();
    blackPerson.birthDay = ui->editBirthday->date();
    blackPerson.address = ui->editAddress->text();
    blackPerson.description = ui->editDescription->text();
    blackPerson.personType = ui->comboPeronType->currentIndex();
    for(int i = 0; i < m_enrollFrameResults.size(); i ++)
        blackPerson.faceData.append(qImage2ByteArray(m_enrollFrameResults[i].faceImage));

    blackInfos.append(blackPerson);

    for(int i = 0; i < m_monitoringAreaModel->rowCount(); i ++)
    {
        QStandardItem* areaNameItem = m_monitoringAreaModel->item(i);
        for(int j = 0; j < areaNameItem->rowCount(); j ++)
        {
            QStandardItem* cameraItem = areaNameItem->child(j);
            if(cameraItem->checkState() == Qt::Checked)
            {
                int selectedServerIndex = m_monitoringAreaInfos[i].serverIndexs[j];
                int selectedChanelIndex = m_monitoringAreaInfos[i].chanelIndexs[j];

                AddBlackInfoSocket* addOneBlackListSocket = new AddBlackInfoSocket;
                m_addOneBlackPersonSockets.append(addOneBlackListSocket);

                //connect(addOneBlackListSocket, SIGNAL(enrollFinished(QObject*)), this, SLOT(slotAddOneBlackPersonFinishded(QObject*)));

                addOneBlackListSocket->setInfo(m_serverInfoSockets[selectedServerIndex]->serverInfo(), selectedChanelIndex, blackInfos);
            }
        }
    }


    if(m_addOneBlackPersonSockets.size() == 0)
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_select_target_camera);
        return;
    }

    for(int i = 0; i < m_addOneBlackPersonSockets.size(); i ++)
        m_addOneBlackPersonSockets[i]->startSocket();

    done(1);
}

void AddBlackListFromCapturedDlg::slotCancel()
{
    done(0);
}

void AddBlackListFromCapturedDlg::slotAddOneBlackPersonFinishded(QObject* obj)
{
//    for(int i = 0; i < m_addOneBlackPersonSockets.size(); i ++)
//    {
//        if(m_addOneBlackPersonSockets[i] == obj)
//        {
//            m_addOneBlackPersonSockets[i]->stopEnroll();
//            delete m_addOneBlackPersonSockets[i];
//            m_addOneBlackPersonSockets.remove(i);
//            break;
//        }
//    }

//    if(m_addOneBlackPersonSockets.size() == 0)
//        done(0);
}

void AddBlackListFromCapturedDlg::retranslateUI()
{
    ui->groupMonitoringArea->setTitle(StringTable::Str_Monitoring_Area);
    ui->groupCapturedImage->setTitle(StringTable::Str_Captured_Image);
    ui->lblFaceDetection->setText(StringTable::Str_Face_Detection);
    ui->lblEnrolledFace->setText(StringTable::Str_Enrolled_Face);
    ui->lblName->setText(StringTable::Str_Name_);
    ui->lblGender->setText(StringTable::Str_Gender_);
    ui->lblPersonType->setText(StringTable::Str_Group + ":");
    ui->lblBirthday->setText(StringTable::Str_Birthday_);
    ui->lblAddress->setText(StringTable::Str_Address_);
    ui->lblDescription->setText(StringTable::Str_Description_);
    ui->btnDown->setText(StringTable::Str__Down);
    ui->btnUp->setText(StringTable::Str__Up);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);

    ui->comboGender->clear();
    ui->comboGender->addItem(StringTable::Str_Male);
    ui->comboGender->addItem(StringTable::Str_Female);

    ui->comboPeronType->clear();
    ui->comboPeronType->addItem(StringTable::Str_White);
    ui->comboPeronType->addItem(StringTable::Str_Black);

    setWindowTitle(StringTable::Str_Add_Black_List);
}
