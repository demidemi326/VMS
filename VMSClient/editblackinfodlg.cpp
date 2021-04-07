#include "editblackinfodlg.h"
#include "ui_editblackinfodlg.h"
#include "nextbuttonitem.h"
#include "frameresultitem.h"
#include "stringtable.h"
#include "editoneblacklistsocket.h"
#include "serverinfosocket.h"
#include "blackfaceadddlg.h"

#define INIT_DETECTION_ITEM_COUNT 50

EditBlackInfoDlg::EditBlackInfoDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditBlackInfoDlg)
{
    ui->setupUi(this);
    retranslateUI();

    m_chanelIndex = -1;
    m_serverInfoSocket = NULL;

    setupActions();
    constructEnrollResultItems();
}

EditBlackInfoDlg::~EditBlackInfoDlg()
{
    delete ui;
}

void EditBlackInfoDlg::setInfo(ServerInfoSocket* serverInfoSocket, int chanelIndex, BLACK_PERSON blackPerson)
{
    m_serverInfoSocket = serverInfoSocket;
    m_chanelIndex = chanelIndex;
    m_blackPersonInfo = blackPerson;

    for(int i = 0; i < m_blackPersonInfo.faceData.size(); i ++)
    {
        FRAME_RESULT frameResult;
        frameResult.faceImage = qByteArray2Image(m_blackPersonInfo.faceData[i]);
        m_enrollFrameResults.append(frameResult);
    }

    refreshEnrollResultItems();
}

BLACK_PERSON EditBlackInfoDlg::blackPersonInfo()
{
    return m_blackPersonInfo;
}

void EditBlackInfoDlg::slotAddFace()
{
    if(INIT_DETECTION_ITEM_COUNT - m_enrollFrameResults.size() <= 0)
        return;

    BlackFaceAddDlg dlg;
    dlg.setInfo(m_serverInfoSocket, INIT_DETECTION_ITEM_COUNT - m_enrollFrameResults.size());
    int ret = dlg.exec();
    if(ret)
    {
        QVector<FRAME_RESULT> addFrameResults = dlg.enrollFrameResults();
        for(int i = 0; i < addFrameResults.size(); i ++)
        {
            m_blackPersonInfo.featData.append(addFrameResults[i].featData);
            m_blackPersonInfo.faceData.append(qImage2ByteArray(addFrameResults[i].faceImage));

            m_enrollFrameResults.append(addFrameResults[i]);
        }

        refreshEnrollResultItems();
    }
}

void EditBlackInfoDlg::slotDeleteFace()
{
    if(m_blackPersonInfo.faceData.size() <= 1)
        return;

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
        if(QMessageBox::question(this, StringTable::Str_Question, StringTable::Str_Are_you_sure_to_delete_selected_face ,QMessageBox::Yes | QMessageBox::No , QMessageBox::No) != QMessageBox::Yes)
            return;

        m_blackPersonInfo.faceData.remove(exist);
        m_blackPersonInfo.featData.remove(exist);
        m_enrollFrameResults.remove(exist);

        refreshEnrollResultItems();
    }
}

void EditBlackInfoDlg::slotOk()
{
    if(ui->editName->text().isEmpty())
    {
        QMessageBox::warning(this, StringTable::Str_Warning, StringTable::Str_Please_input_a_name);
        return;
    }

    m_blackPersonInfo.name = ui->editName->text();
    m_blackPersonInfo.address = ui->editAddress->text();
    m_blackPersonInfo.description = ui->editDescription->text();
    m_blackPersonInfo.personType = ui->comboPersonType->currentIndex();
    m_blackPersonInfo.birthDay = ui->editBirthday->date();
    m_blackPersonInfo.gender = ui->comboGender->currentIndex();
    m_blackPersonInfo.featData = m_blackPersonInfo.featData;
    m_blackPersonInfo.faceData = m_blackPersonInfo.faceData;

    done(1);
}

void EditBlackInfoDlg::slotCancel()
{
    done(0);
}


void EditBlackInfoDlg::setupActions()
{
    m_enrollScene = new QGraphicsScene;
    ui->viewEnroll->setScene(m_enrollScene);
    ui->viewEnroll->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->viewEnroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->viewEnroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->viewEnroll->horizontalScrollBar()->setSingleStep(FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP);


    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(ui->btnAddFace, SIGNAL(clicked()), this, SLOT(slotAddFace()));
    connect(ui->btnDeleteFace, SIGNAL(clicked()), this, SLOT(slotDeleteFace()));
}


void EditBlackInfoDlg::slotEnrollPrevClicked()
{
    int curVal = ui->viewEnroll->horizontalScrollBar()->value();
    qDebug() << ui->viewEnroll->horizontalScrollBar()->pageStep() << ui->viewEnroll->horizontalScrollBar()->singleStep();
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

void EditBlackInfoDlg::slotEnrollNextClicked()
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

void EditBlackInfoDlg::resizeEvent(QResizeEvent* e)
{
    QDialog::resizeEvent(e);
    relocateEnrollResultItems();
}

void EditBlackInfoDlg::constructEnrollResultItems()
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

void EditBlackInfoDlg::refreshEnrollResultItems()
{
    if(m_enrollFrameResults.size() > m_enrollFrameItems.size())
    {
        int addCount = m_enrollFrameResults.size() - m_enrollFrameItems.size();
        for(int i = 0; i < addCount; i ++)
        {
            FrameResultItem* frameResultItem = new FrameResultItem;
            m_enrollScene->addItem(frameResultItem);
            m_enrollFrameItems.append(frameResultItem);
        }
    }

    for(int i = 0; i < m_enrollFrameItems.size(); i ++)
        m_enrollFrameItems[i]->setEmpty();

    for(int i = 0; i < m_enrollFrameResults.size(); i ++)
        m_enrollFrameItems[i]->setInfo(m_enrollFrameResults[i]);

    relocateEnrollResultItems();

    ui->editName->setText(m_blackPersonInfo.name);
    ui->editAddress->setText(m_blackPersonInfo.address);
    ui->editDescription->setText(m_blackPersonInfo.description);
    ui->editBirthday->setDate(m_blackPersonInfo.birthDay);
    ui->comboGender->setCurrentIndex(m_blackPersonInfo.gender);
    ui->comboPersonType->setCurrentIndex(m_blackPersonInfo.personType);
}

void EditBlackInfoDlg::relocateEnrollResultItems()
{
    QRect sceneRect(0, 0, NEXT_BUTTON_ITEM_WIDTH * 2 + m_enrollFrameItems.size() * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), FRAME_RESULT_ITEM_HEIGHT);
    m_enrollScene->setSceneRect(sceneRect);
    for(int i = 0; i < m_enrollFrameItems.size(); i ++)
        m_enrollFrameItems[i]->setPos(NEXT_BUTTON_ITEM_WIDTH + i * (FRAME_RESULT_ITEM_WIDTH + FRAME_RESULT_ITEM_GAP), 0);

    m_enrollPrevItem->setPos(0, 0);
    m_enrollNextItem->setPos(ui->viewEnroll->width() - NEXT_BUTTON_ITEM_WIDTH, 0);

    m_enrollScene->update();
}


void EditBlackInfoDlg::retranslateUI()
{
    ui->lblEnrolledFace->setText(StringTable::Str_Enrolled_Face);
    ui->lblName->setText(StringTable::Str_Name_);
    ui->lblGender->setText(StringTable::Str_Gender_);
    ui->lblPersonType->setText(StringTable::Str_Group + ":");
    ui->lblBirthday->setText(StringTable::Str_Birthday_);
    ui->lblAddress->setText(StringTable::Str_Address_);
    ui->lblDescription->setText(StringTable::Str_Description_);
    ui->btnAddFace->setText(StringTable::Str_Add_Face);
    ui->btnDeleteFace->setText(StringTable::Str_Delete_Face);
    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);

    ui->comboGender->clear();
    ui->comboGender->addItem(StringTable::Str_Male);
    ui->comboGender->addItem(StringTable::Str_Female);

    ui->comboPersonType->clear();
    ui->comboPersonType->addItem(StringTable::Str_White);
    ui->comboPersonType->addItem(StringTable::Str_Black);

    setWindowTitle(StringTable::Str_Edit_Black_List);

}
