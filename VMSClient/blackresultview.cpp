#include "blackresultview.h"
#include "ui_blackresultview.h"
#include "clientbase.h"
#include "stringtable.h"

#include <QtWidgets>

#define BLACK_TITLE_HEIGHT 50
#define BLACK_SUB_TITLE_HEIGHT BLACK_TITLE_HEIGHT / 2

BlackResultView::BlackResultView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BlackResultView)
{
    ui->setupUi(this);
}

BlackResultView::~BlackResultView()
{
    delete ui;
}

void BlackResultView::setBlackResult(QVector<BLACK_RECOG_RESULT> blackResult)
{
    m_blackResult = blackResult;
    if(m_blackResult.size())
    {
        m_galleryImage = qByteArray2Image(m_blackResult[0].galleryFaceData);
        m_probeImage = qByteArray2Image(m_blackResult[0].probeFaceData);
    }
    else
    {
        m_galleryImage = QImage();
        m_probeImage = QImage();
    }
    update();
}

void BlackResultView::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    if(m_blackResult.size())
    {
        QPainter painter;
        painter.begin(this);

        QRect probeRect(0, BLACK_TITLE_HEIGHT, rect().width() / 2, rect().height() - BLACK_TITLE_HEIGHT);
        QRect probeFrameRect = getFrameRect(probeRect, m_probeImage.size());

        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(probeFrameRect, m_probeImage);

        QRect galleryRect(rect().width() / 2, BLACK_TITLE_HEIGHT, rect().width() / 2, rect().height() - BLACK_TITLE_HEIGHT);
        QRect galleryFrameRect = getFrameRect(galleryRect, m_galleryImage.size());

        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(galleryFrameRect, m_galleryImage);

        painter.save();
        if(m_blackResult[0].personType == 0)
            painter.setPen(QPen(QColor(0, 255, 0), 3));
        else
            painter.setPen(QPen(QColor(255, 0, 0), 3));
        painter.drawRect(QRect(galleryFrameRect.left(), galleryFrameRect.top(), galleryFrameRect.width() - 2, galleryFrameRect.height() - 2));
        painter.restore();

        QString scoreStr;
        scoreStr.sprintf("%s : %.3f", StringTable::Str_Similiarity.toUtf8().data(), m_blackResult[0].similiarity);
        QRect scoreRect(0, qMin(probeFrameRect.top() - BLACK_TITLE_HEIGHT, galleryFrameRect.top() - BLACK_TITLE_HEIGHT), rect().width(), BLACK_SUB_TITLE_HEIGHT);

        QRect probeTitleRect(0, scoreRect.bottom(), rect().width() / 2, BLACK_SUB_TITLE_HEIGHT);
        QRect galleryTitleRect(rect().width() / 2, scoreRect.bottom() , rect().width() / 2, BLACK_SUB_TITLE_HEIGHT);

        painter.save();
        painter.setPen(Qt::red);
        QFont scoreFont = painter.font();
        scoreFont.setPointSize(12);
        painter.setFont(scoreFont);
        painter.drawText(scoreRect, Qt::AlignHCenter | Qt::AlignVCenter, scoreStr);
        painter.restore();

        painter.drawText(probeTitleRect, Qt::AlignHCenter | Qt::AlignVCenter, StringTable::Str_Captured);
        painter.drawText(galleryTitleRect, Qt::AlignHCenter | Qt::AlignVCenter, StringTable::Str_Black_DB);

        painter.end();
    }
}
