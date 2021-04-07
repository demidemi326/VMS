#include "blackenrollbatchdlg.h"
#include "ui_blackenrollbatchdlg.h"
#include "stringtable.h"
#include "imagesearchengine.h"
#include "imageprocessingsocket.h"
#include "serverinfosocket.h"
#include "clientbase.h"
#include "editblackinfodlg.h"

#include <QtWidgets>

BlackEnrollBatchDlg::BlackEnrollBatchDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlackEnrollBatchDlg)
{
    ui->setupUi(this);

    m_serverInfoSocket = NULL;

    m_toolbar = new QToolBar;
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toolbar->addAction(ui->actionSelectFiles);
    m_toolbar->addAction(ui->actionSelectFolders);
    m_toolbar->addSeparator();
    m_toolbar->addAction(ui->actionEditBlackBatch);
    m_toolbar->addAction(ui->actionDeleteBlackBatch);

    m_lblPersonType = new QLabel;
    m_lblPersonType->setText(StringTable::Str_Group + ": ");
    m_lblPersonType->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_comboPersonType = new QComboBox;
    m_comboPersonType->addItem(StringTable::Str_White);
    m_comboPersonType->addItem(StringTable::Str_Black);
    m_toolbar->addWidget(m_lblPersonType);
    m_toolbar->addWidget(m_comboPersonType);

    QVBoxLayout* toolbarLayout = new QVBoxLayout;
    toolbarLayout->addWidget(m_toolbar);
    toolbarLayout->setMargin(0);
    toolbarLayout->setSpacing(0);
    ui->widToolBar->setLayout(toolbarLayout);

    m_batchModel = new QStandardItemModel(0, 6);
    m_batchModel->setHeaderData(0, Qt::Horizontal, StringTable::Str_Name);
    m_batchModel->setHeaderData(1, Qt::Horizontal, StringTable::Str_Group);
    m_batchModel->setHeaderData(2, Qt::Horizontal, StringTable::Str_Gender);
    m_batchModel->setHeaderData(3, Qt::Horizontal, StringTable::Str_Birthday);
    m_batchModel->setHeaderData(4, Qt::Horizontal, StringTable::Str_Address);
    m_batchModel->setHeaderData(5, Qt::Horizontal, StringTable::Str_Description);

    m_batchSelectionModel = new QItemSelectionModel(m_batchModel);

    ui->viewBlackBatchList->setModel(m_batchModel);
    ui->viewBlackBatchList->setSelectionModel(m_batchSelectionModel);
    ui->viewBlackBatchList->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->viewBlackBatchList->setSelectionMode( QAbstractItemView::ContiguousSelection );
    ui->viewBlackBatchList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->viewBlackBatchList->setGridStyle(Qt::NoPen);
    ui->viewBlackBatchList->setAlternatingRowColors(true);
    ui->viewBlackBatchList->verticalHeader()->setDefaultSectionSize(20);
    ui->viewBlackBatchList->verticalHeader()->setHighlightSections(false);
    ui->viewBlackBatchList->horizontalHeader()->setHighlightSections(false);

    m_imageSearchEngine = new ImageSearchEngine;
    m_imageProcessingSocket = new ImageProcessingSocket;

    m_progressDialog = new QProgressDialog(this);
    m_progressDialog->setModal(true);

    connect(ui->actionSelectFiles, SIGNAL(triggered()), this, SLOT(slotSelectFiles()));
    connect(ui->actionSelectFolders, SIGNAL(triggered()), this, SLOT(slotSelectFolders()));
    connect(ui->actionEditBlackBatch, SIGNAL(triggered()), this, SLOT(slotEditBlackBatch()));
    connect(ui->actionDeleteBlackBatch, SIGNAL(triggered()), this, SLOT(slotDeleteBlackBatch()));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    connect(m_imageSearchEngine, SIGNAL(searchResults(QStringList)), this, SLOT(addImageFiles(QStringList)));
    connect(m_imageProcessingSocket, SIGNAL(receiveFrameResults(QVector<FRAME_RESULT>)), this, SLOT(slotReceivedFrameResults(QVector<FRAME_RESULT>)));

    connect(ui->viewBlackBatchList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(blackListItemDoubleClicked(QModelIndex)));
    connect(m_batchSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(blackListItemSelectionChanged(QItemSelection, QItemSelection)));

    connect(m_comboPersonType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPersonTypeChanged(int)));
    connect(m_progressDialog, SIGNAL(canceled()), this, SLOT(slotProgressCanceled()));

    refreshActions();
    retranslateUI();
}

BlackEnrollBatchDlg::~BlackEnrollBatchDlg()
{
    delete ui;
}

void BlackEnrollBatchDlg::setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex)
{
    m_serverInfoSocket = serverInfoSocket;
    m_chanelIndex = chanelIndex;
}

QVector<BLACK_PERSON> BlackEnrollBatchDlg::blackPersonInfos()
{
    return m_batchBlackList;
}

void BlackEnrollBatchDlg::slotSelectFiles()
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QStringList fileNames = QFileDialog::getOpenFileNames(this, StringTable::Str_Open_File, setting.value("last enroll black files dir").toString(),
                                                    tr("Image Files(*.bmp *.jpg *.jpeg *.tif *.tiff)"));
    if(fileNames.size() == 0)
        return;

    QFileInfo info(fileNames[0]);
    setting.setValue("last enroll black files dir", info.absoluteDir().absolutePath());

    addImageFiles(fileNames);
}

void BlackEnrollBatchDlg::slotSelectFolders()
{
    QSettings setting(QSettings::IniFormat, QSettings::SystemScope, ORG_NAME, APP_NAME);
    QString folderPath = QFileDialog::getExistingDirectory(this, StringTable::Str_Open_Folder,
                                                    setting.value("last enroll black folder dir").toString(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if(folderPath.isEmpty())
        return;

    m_imageSearchEngine->stopEngine();
    setting.setValue("last enroll black folder dir", folderPath);

    m_imageSearchEngine->searchImages(folderPath);
}

void BlackEnrollBatchDlg::slotEditBlackBatch()
{
    if(!m_batchSelectionModel->hasSelection())
        return;

    QModelIndex selectedIndex = m_batchSelectionModel->selection().indexes().at(0);
    QString selectedName = m_batchModel->data(m_batchModel->index(selectedIndex.row(), 0)).toString();

    int exist = -1;
    for(int i = 0; i < m_batchBlackList.size(); i ++)
    {
        if(m_batchBlackList[i].name == selectedName)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
        return;

    EditBlackInfoDlg editBlackInfoDlg;
    editBlackInfoDlg.setInfo(m_serverInfoSocket, m_chanelIndex, m_batchBlackList[exist]);
    int ret = editBlackInfoDlg.exec();
    if(ret == 1)
    {
        m_batchBlackList[exist] = editBlackInfoDlg.blackPersonInfo();
        refreshBlackBatchList();
    }
}

void BlackEnrollBatchDlg::slotDeleteBlackBatch()
{
    QVector<int> deleteIndexs;
    QModelIndexList selectedIndexs = m_batchSelectionModel->selectedIndexes();
    for(int i = 0; i < selectedIndexs.size(); i ++)
    {
        int exist = -1;
        for(int j = 0; j < deleteIndexs.size(); j ++)
        {
            if(deleteIndexs[j] == selectedIndexs[i].row())
            {
                exist = j;
                break;
            }
        }

        if(exist < 0)
            deleteIndexs.append(selectedIndexs[i].row());
    }

    if(deleteIndexs.size() < 0)
        return;

    if(QMessageBox::question(this, StringTable::Str_Question, StringTable::Str_Are_you_sure_to_delete_selected_black_person_infos ,QMessageBox::Yes | QMessageBox::No , QMessageBox::No) != QMessageBox::Yes)
        return;

    for(int i = deleteIndexs.size() - 1; i >= 0; i --)
        m_batchBlackList.removeAt(deleteIndexs[i]);

    refreshBlackBatchList();
}

void BlackEnrollBatchDlg::slotProcessBlackBatch()
{
    if(m_serverInfoSocket == NULL)
        return;

    m_imageProcessingSocket->stopProcessing();
    m_processingIndex = 0;

    m_progressDialog->reset();
    m_progressDialog->show();

    processCurrentImage();
}

void BlackEnrollBatchDlg::slotOk()
{
    m_imageProcessingSocket->stopProcessing();

    done(1);
}

void BlackEnrollBatchDlg::slotCancel()
{
    m_imageProcessingSocket->stopProcessing();

    done(0);
}

void BlackEnrollBatchDlg::addImageFiles(QStringList fileNames)
{
    for(int i = 0; i < fileNames.size(); i ++)
    {
        QFileInfo fileInfo(fileNames[i]);

        int exist = -1;
        for(int j = 0; j < m_batchBlackList.size(); j ++)
        {
            if(m_batchBlackList[j].name == fileInfo.baseName())
            {
                exist = j;
                break;
            }
        }

        if(exist >= 0)
            continue;

        m_batchFiles.append(fileNames[i]);
    }

    slotProcessBlackBatch();
}

void BlackEnrollBatchDlg::processCurrentImage()
{
    if(!m_progressDialog->wasCanceled())
    {
        m_progressDialog->setMaximum(m_batchFiles.size());
        m_progressDialog->setValue(m_processingIndex + 1);
        if(m_processingIndex >= 0 && m_processingIndex < m_batchFiles.size())
        {
            QImage processingImage(m_batchFiles[m_processingIndex]);
            if(processingImage.isNull())
            {
                m_processingIndex ++;
                processCurrentImage();
                return;
            }

            m_imageProcessingSocket->startProcessing(m_serverInfoSocket->serverInfo(), processingImage);
        }
        else
        {
            m_batchFiles.clear();

            refreshBlackBatchList();
            m_progressDialog->close();
        }
    }
    else
    {
        m_batchFiles.clear();
        m_progressDialog->close();
    }
}

void BlackEnrollBatchDlg::slotReceivedFrameResults(QVector<FRAME_RESULT> frameResults)
{
    QFileInfo fileInfo(m_batchFiles[m_processingIndex]);
    int addIndex = m_batchBlackList.size();

    BLACK_PERSON blackPerson;
    blackPerson.gender = 0;
    blackPerson.name = fileInfo.baseName();
    blackPerson.personType = m_comboPersonType->currentIndex();

    m_batchBlackList.append(blackPerson);

    if(frameResults.size())
    {
        m_batchBlackList[addIndex].faceData.append(qImage2ByteArray(frameResults[0].faceImage));
        m_batchBlackList[addIndex].featData.append(frameResults[0].featData);
    }

    m_processingIndex ++;
    processCurrentImage();
}

void BlackEnrollBatchDlg::blackListItemDoubleClicked(QModelIndex)
{
    slotEditBlackBatch();
}

void BlackEnrollBatchDlg::blackListItemSelectionChanged(QItemSelection, QItemSelection)
{
    refreshActions();
    if(!m_batchSelectionModel->hasSelection())
    {
        ui->lblBlackInfoFace->setPixmap(QPixmap());
        return;
    }

    QModelIndex selectedIndex = m_batchSelectionModel->selection().indexes().at(0);
    QString selectedName = m_batchModel->data(m_batchModel->index(selectedIndex.row(), 0)).toString();

    int exist = -1;
    for(int i = 0; i < m_batchBlackList.size(); i ++)
    {
        if(m_batchBlackList[i].name == selectedName)
        {
            exist = i;
            break;
        }
    }

    if(exist < 0)
    {
        ui->lblBlackInfoFace->setPixmap(QPixmap());
        return;
    }

    if(m_batchBlackList[exist].faceData.size())
        ui->lblBlackInfoFace->setPixmap(QPixmap::fromImage(qByteArray2Image(m_batchBlackList[exist].faceData[0])));
    else
        ui->lblBlackInfoFace->setPixmap(QPixmap());
}

void BlackEnrollBatchDlg::refreshBlackBatchList()
{
    m_batchModel->removeRows(0, m_batchModel->rowCount());

    for(int i = 0; i < m_batchBlackList.size(); i ++)
    {
        m_batchModel->insertRow(m_batchModel->rowCount());
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 0), m_batchBlackList[i].name);
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 0), Qt::AlignCenter, Qt::TextAlignmentRole);

        QString personTypeStr;
        if(m_batchBlackList[i].personType == 0)
        {
            personTypeStr = StringTable::Str_White;
            m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 0), QColor(Qt::green), Qt::BackgroundColorRole);
        }
        else if(m_batchBlackList[i].personType == 1)
        {
            personTypeStr = StringTable::Str_Black;
            m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 0), QColor(Qt::red), Qt::BackgroundColorRole);
        }
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 1), personTypeStr);
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 1), Qt::AlignCenter, Qt::TextAlignmentRole);


        QString genderStr;
        if(m_batchBlackList[i].gender == 0)
            genderStr = StringTable::Str_Male;
        else if(m_batchBlackList[i].gender == 1)
            genderStr = StringTable::Str_Female;
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 2), genderStr);
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 2), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 3), m_batchBlackList[i].birthDay);
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 3), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 4), m_batchBlackList[i].address);
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 4), Qt::AlignCenter, Qt::TextAlignmentRole);

        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 5), m_batchBlackList[i].description);
        m_batchModel->setData(m_batchModel->index(m_batchModel->rowCount() - 1, 5), Qt::AlignCenter, Qt::TextAlignmentRole);

        if(m_batchBlackList[i].featData.size())
            m_batchModel->item(m_batchModel->rowCount() - 1, 0)->setIcon(QIcon(":/images/check.png"));
        else
            m_batchModel->item(m_batchModel->rowCount() - 1, 0)->setIcon(QIcon(":/images/empty.png"));
    }

    refreshActions();
}

void BlackEnrollBatchDlg::refreshActions()
{
    if(m_batchSelectionModel->hasSelection())
    {
        ui->actionEditBlackBatch->setEnabled(true);
        ui->actionDeleteBlackBatch->setEnabled(true);
    }
    else
    {
        ui->actionEditBlackBatch->setEnabled(false);
        ui->actionDeleteBlackBatch->setEnabled(false);
    }
}

void BlackEnrollBatchDlg::resizeEvent(QResizeEvent* e)
{
    QDialog::resizeEvent(e);

    ui->viewBlackBatchList->setColumnWidth(0, 80);
    ui->viewBlackBatchList->setColumnWidth(1, 80);
    ui->viewBlackBatchList->setColumnWidth(2, 80);
    ui->viewBlackBatchList->setColumnWidth(3, 120);
    ui->viewBlackBatchList->setColumnWidth(4, 120);
    ui->viewBlackBatchList->setColumnWidth(5, 120);
}

void BlackEnrollBatchDlg::retranslateUI()
{
    ui->actionSelectFiles->setText(StringTable::Str_Add_Files);
    ui->actionSelectFolders->setText(StringTable::Str_Add_Folder);
    ui->actionEditBlackBatch->setText(StringTable::Str_Edit);
    ui->actionDeleteBlackBatch->setText(StringTable::Str_Delete);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);

    m_progressDialog->setCancelButtonText(StringTable::Str_Cancel);
    m_progressDialog->setWindowTitle(StringTable::Str_Enroll_Batch);
    setWindowTitle(StringTable::Str_Enroll_Batch);
}

void BlackEnrollBatchDlg::slotPersonTypeChanged(int personType)
{
    for(int i = 0; i < m_batchBlackList.size(); i ++)
        m_batchBlackList[i].personType = personType;

    refreshBlackBatchList();
}

void BlackEnrollBatchDlg::slotProgressCanceled()
{
    refreshBlackBatchList();
}
