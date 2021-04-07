#include "resultalarmdlg.h"
#include "ui_resultalarmdlg.h"
#include "stringtable.h"

#include <QtWidgets>

ResultAlarmDlg::ResultAlarmDlg(QWidget *parent) :
    QWidget(parent, Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint),
    ui(new Ui::ResultAlarmDlg)
{
    ui->setupUi(this);
    m_alarmId = -1;

    connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(slotClose()));
}

ResultAlarmDlg::~ResultAlarmDlg()
{
    delete ui;
}

void ResultAlarmDlg::setData(int alrmId, QVector<BLACK_RECOG_RESULT> blackRecogResult)
{
    m_blackRecogResult = blackRecogResult;
    m_alarmId = alrmId;

    QString scoreStr;
    scoreStr.sprintf("%s : %f", StringTable::Str_Similiarity.toUtf8().data(), blackRecogResult[0].similiarity);
    ui->lblScore->setText("<font color='#FF0000'>" + scoreStr + "</font>");

    QString personTypeStr;
    if(blackRecogResult[0].personType == 0)
        personTypeStr = StringTable::Str_White;
    else if(blackRecogResult[0].personType == 1)
        personTypeStr = StringTable::Str_Black;

    QString genderStr;
    if(blackRecogResult[0].gender == 0)
        genderStr = StringTable::Str_Male;
    else
        genderStr = StringTable::Str_Female;


    ui->editName->setText(blackRecogResult[0].name);
    ui->editGroup->setText(personTypeStr);
    ui->editDescription->setText(blackRecogResult[0].description);
    ui->editArea->setText(blackRecogResult[0].areaName);
    ui->editDateTime->setText(blackRecogResult[0].dateTime.toString("yyyy-MM-dd hh:mm:ss"));
    ui->editGender->setText(genderStr);
    ui->editBirthDay->setText(blackRecogResult[0].birthday.toString("yyyy-MM-dd"));
    ui->editAddress->setText(blackRecogResult[0].address);

    updateFaceImages();

    show();
}


void ResultAlarmDlg::updateFaceImages()
{
    if(m_blackRecogResult.size())
    {
        QImage probeImage = qByteArray2Image(m_blackRecogResult[0].probeFaceData);
        QImage galleryImage = qByteArray2Image(m_blackRecogResult[0].galleryFaceData);

        QRect probeImageRect = getFrameRect(ui->lblCaptured->rect(), probeImage.size());
        QImage probeScaledImage = probeImage.scaled(probeImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QRect galleryImageRect = getFrameRect(ui->lblBlackList->rect(), galleryImage.size());
        QImage galleryScaledImage = galleryImage.scaled(galleryImageRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QPainter painter;
        painter.begin(&galleryScaledImage);
        if(m_blackRecogResult[0].personType == 0)
            painter.setPen(QPen(QColor(0, 255, 0), 3));
        else
            painter.setPen(QPen(QColor(255, 0, 0), 3));
        painter.drawRect(QRect(1, 1, galleryScaledImage.size().width() - 3, galleryScaledImage.size().height() - 3));
        painter.end();


        ui->lblCaptured->setPixmap(QPixmap::fromImage(probeScaledImage));
        ui->lblBlackList->setPixmap(QPixmap::fromImage(galleryScaledImage));
    }
}

void ResultAlarmDlg::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    updateFaceImages();
}

void ResultAlarmDlg::closeEvent(QCloseEvent * e)
{
    e->ignore();
    m_alarmId = -1;
    hide();
    emit closed();
}

void ResultAlarmDlg::slotClose()
{
    m_alarmId = -1;
    hide();
    emit closed();
}

void ResultAlarmDlg::paintEvent(QPaintEvent *)
{
    updateFaceImages();
}

void ResultAlarmDlg::retranslateUI()
{
    ui->lblName->setText(StringTable::Str_Name_);
    ui->lblGender->setText(StringTable::Str_Gender_);
    ui->lblPersonType->setText(StringTable::Str_Group + ":");
    ui->lblBirthday->setText(StringTable::Str_Birthday_);
    ui->lblAddress->setText(StringTable::Str_Address_);
    ui->lblDescription->setText(StringTable::Str_Description_);
    ui->lblArea->setText(StringTable::Str_Monitoring_Area + ":");
    ui->lblDateTime->setText(StringTable::Str_Date_Time);
    ui->groupCaptured->setTitle(StringTable::Str_Captured);
    ui->groupBlackList->setTitle(StringTable::Str_Black_DB);
    ui->btnClose->setText(StringTable::Str_Close);

    setWindowTitle(StringTable::Str_Alarm);
}
