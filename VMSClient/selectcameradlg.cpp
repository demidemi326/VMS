#include "selectcameradlg.h"
#include "ui_selectcameradlg.h"
#include "ipcameradevice.h"
#include "searchcameraitem.h"
#include "ipcamera.h"
#include "stringtable.h"

#include <QtWidgets>

#define DEVICE_LIST_TOP_MARGIN 15
#define DEVICE_LIST_LEFT_MARGIN 5
#define DEVICE_LIST_SPACING 10

extern void convertYUV420P_toRGB888_Scale4(unsigned char* data, int width, int height, unsigned char* dstData);

SelectCameraDlg::SelectCameraDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectCameraDlg)
{
    ui->setupUi(this);

    m_deviceSearcher = new DeviceSearcher;
    m_ipCamera = new IpCamera;

    ui->progressBar->hide();

    m_scene = new QGraphicsScene;
    ui->viewCameraList->setScene(m_scene);
    ui->viewCameraList->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->viewCameraList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QRegExp ipRegExp("([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))([.])([0-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))");
    QValidator *ipValidator = new QRegExpValidator(ipRegExp, ui->editIpAddress);
    ui->editIpAddress->setValidator(ipValidator);

    QRegExp portRegExp("([0-9])([0-9])([0-9])([0-9])([0-9])");
    QValidator *portValidator = new QRegExpValidator(portRegExp, ui->editPortNum);
    ui->editPortNum->setValidator(portValidator);

    ui->btnOk->setText(StringTable::Str_Ok);
    ui->btnCancel->setText(StringTable::Str_Cancel);
    ui->btnRefresh->setText(StringTable::Str_Search);
    ui->groupBox_2->setTitle(StringTable::Str_Preview);
    ui->groupBox->setTitle(StringTable::Str_Select_Camera);
    ui->lblIpAddress->setText(StringTable::Str_IP_Address_);
    ui->lblPassword->setText(StringTable::Str_Password_);
    ui->lblUserName->setText(StringTable::Str_User_Name);
    ui->lblPortNum->setText(StringTable::Str_Port_Num_);
    setWindowTitle(StringTable::Str_Select_Camera);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(ui->btnRefresh, SIGNAL(clicked()), this, SLOT(slotRefresh()));

    connect(m_deviceSearcher, SIGNAL(receiveData(QHash<QString,QString>)), this, SLOT(slotReceiveDeviceData(QHash<QString,QString>)));
    connect(m_scene, SIGNAL(selectionChanged()), this, SLOT(slotCameraSelectionChanged()));
    connect(m_ipCamera, SIGNAL(frameChanged()), this, SLOT(slotFrameChanged()));

    QSettings settings;
    ui->editIpAddress->setText(settings.value("search camera ip").toString());
    ui->editUserName->setText(settings.value("search camera user").toString());
    ui->editPassword->setText(settings.value("search camera password").toString());
    ui->editPortNum->setText(settings.value("search camera port").toString());

    slotCameraSelectionChanged();
}

SelectCameraDlg::~SelectCameraDlg()
{
    delete ui;
}


QHash<QString, QString> SelectCameraDlg::cameraInfos()
{
    QHash<QString, QString> cameraInfos;
    QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
    if(selectedItems.size() == 0)
        return cameraInfos;

    SearchCameraItem* cameraItem = (SearchCameraItem*)selectedItems[0];
    cameraInfos = cameraItem->info();
    return cameraInfos;
}

void SelectCameraDlg::slotOk()
{
    done(1);
}

void SelectCameraDlg::slotCancel()
{
    done(0);
}

void SelectCameraDlg::slotRefresh()
{
    QSettings settings;
    settings.setValue("search camera ip", ui->editIpAddress->text());
    settings.setValue("search camera user", ui->editUserName->text());
    settings.setValue("search camera password", ui->editPassword->text());
    settings.setValue("search camera port", ui->editPortNum->text());

    m_scene->clear();
    m_cameraSevices.clear();

    QString deviceServiceStr;

    if(ui->editPortNum->text().isEmpty())
        deviceServiceStr.sprintf("http://%s/onvif/device_service", ui->editIpAddress->text().toUtf8().data());
    else
        deviceServiceStr.sprintf("http://%s:%s/onvif/device_service", ui->editIpAddress->text().toUtf8().data(), ui->editPortNum->text().toUtf8().data());

    IpCameraDevice* ipCameraDevice = new IpCameraDevice;
    connect(ipCameraDevice, SIGNAL(findNewCamera(QHash<QString,QString>)), this, SLOT(slotFindNewCamera(QHash<QString,QString>)));

    ui->btnRefresh->setEnabled(false);
    ui->progressBar->show();
    ipCameraDevice->setDeviceService(deviceServiceStr, ui->editIpAddress->text(), ui->editUserName->text(), ui->editPassword->text());
    ui->progressBar->hide();
    ui->btnRefresh->setEnabled(true);
}

void SelectCameraDlg::slotReceiveDeviceData(QHash<QString,QString> receiveData)
{
    QString serviceAddr = receiveData["device_service_address"];
    QStringList serviceList = serviceAddr.split(" ");
    if(serviceList.size() == 0)
        return;

    for(int i = 0; i < m_cameraSevices.size(); i ++)
    {
        if(m_cameraSevices[i] == serviceList[0])
            return;
    }

    m_cameraSevices.append(serviceList[0]);

    IpCameraDevice* ipCameraDevice = new IpCameraDevice;
    connect(ipCameraDevice, SIGNAL(findNewCamera(QHash<QString,QString>)), this, SLOT(slotFindNewCamera(QHash<QString,QString>)));

    ui->btnRefresh->setEnabled(false);
    ui->progressBar->show();
    ipCameraDevice->setDeviceService(serviceList[0], ui->editIpAddress->text(), ui->editUserName->text(), ui->editPassword->text());
    ui->progressBar->hide();
    ui->btnRefresh->setEnabled(true);

    delete ipCameraDevice;
}

void SelectCameraDlg::slotFindNewCamera(QHash<QString, QString> deviceInfos)
{
    qDebug() << "find new camera" << deviceInfos;

    QList<QGraphicsItem*> cameraItems = m_scene->items();
    for(int i = 0; i < cameraItems.size(); i ++)
    {
        SearchCameraItem* cameraItem = (SearchCameraItem*)cameraItems[i];
        if(cameraItem->isEqual(deviceInfos))
            return;
    }

    SearchCameraItem* cameraItem = new SearchCameraItem;
    cameraItem->setInfo(deviceInfos);
    cameraItem->setPos(DEVICE_LIST_LEFT_MARGIN, cameraItems.size() * (DEVICE_LIST_SPACING + SEARCH_CAMERA_ITEM_HEIGHT) + DEVICE_LIST_TOP_MARGIN);

    m_scene->addItem(cameraItem);

    m_scene->setSceneRect(QRect(0, 0, DEVICE_LIST_LEFT_MARGIN + SEARCH_CAMERA_ITEM_WIDTH,
                                m_scene->items().size() * SEARCH_CAMERA_ITEM_HEIGHT + DEVICE_LIST_TOP_MARGIN));

    m_scene->update();
}

void SelectCameraDlg::slotCameraSelectionChanged()
{
    QList<QGraphicsItem*> selectedItems = m_scene->selectedItems();
    if(selectedItems.size() == 0)
    {
        ui->btnOk->setEnabled(false);

        m_ipCamera->close();
        ui->viewPreview->setImage(QImage(), 0, 0);
    }
    else
    {
        SearchCameraItem* cameraItem = (SearchCameraItem*)selectedItems[0];
        QHash<QString, QString> cameraInfos = cameraItem->info();
        m_ipCamera->open(cameraInfos["streamuri"]);

        ui->btnOk->setEnabled(true);
    }
}

void SelectCameraDlg::slotFrameChanged()
{
    QByteArray yuvData;
    int frameWidth, frameHeight;
    qint64 frameTime;
    m_ipCamera->getCurFrame(yuvData, frameWidth, frameHeight, frameTime);
    if(yuvData.size() == 0)
        return;

    QImage image(frameWidth / 2, frameHeight / 2, QImage::Format_RGB888);
    convertYUV420P_toRGB888_Scale4((unsigned char*)yuvData.data(), frameWidth, frameHeight, image.bits());

    ui->viewPreview->setImage(image, frameWidth, frameHeight);
}
