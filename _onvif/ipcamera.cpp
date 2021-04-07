#include "ipcamera.h"
#include "mediamanagement.h"

#include <QtWidgets>
#include <process.h>

#define TIME_OUT 2 * 1000


IpCamera::IpCamera(QObject *parent) :
    QThread(parent)
{
    m_fmt_ctx = NULL;
    m_dec_ctx = NULL;
    m_frame = NULL;
    m_video_stream_index = -1;
    m_running = 0;
    m_index = 0;
    m_mdediaManagement = NULL;
    m_sender = 0;
    m_receiveIFrame = 0;
    m_reconnect = 1;
    memset(&m_curFrame, 0, sizeof(m_curFrame));
    memset(m_frameData, 0, sizeof(m_frameData));

    connect(this, SIGNAL(requestReconnect()), this, SLOT(start()));
}

IpCamera::~IpCamera()
{
}

void IpCamera::init()
{
    avcodec_register_all();
    av_register_all();
    avfilter_register_all();
    avformat_network_init();
}

bool IpCamera::open(QString streamUri)
{
    if(streamUri.isEmpty())
        return false;

    close();

    m_streamUrl = streamUri;

    start();
}

void IpCamera::close()
{
    setRunning(0);
    wait();
}

int IpCamera::isRunning()
{
    m_mutex.lock();
    int running = m_running;
    m_mutex.unlock();

    return running;
}

void IpCamera::setRunning(int running)
{
    m_mutex.lock();
    m_running = running;
    m_mutex.unlock();
}

void IpCamera::setReconnect(int reconnect)
{
    m_reconnect = reconnect;
}

void IpCamera::getCurFrame(QByteArray& frameData, int& width, int& height, qint64& imageTime)
{
    QMutexLocker locker(&m_frameMutex);
    if(m_curFrame.width == 0)
        return;

    frameData = QByteArray((char*)m_curFrame.yuvData, m_curFrame.width * m_curFrame.height * 1.5);
    imageTime = m_curFrame.frameTime;
    width = m_curFrame.width;
    height = m_curFrame.height;

    m_curFrame.width = 0;
}

YUVFrame IpCamera::readFrame()
{
    int ret, got_frame = 0;
    YUVFrame nullFrame = { 0 };

    if ((ret = av_read_frame(m_fmt_ctx, &m_packet)) < 0)
        return nullFrame;

    qint64 frameTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if(m_packet.stream_index != m_video_stream_index)
    {
        av_free_packet(&m_packet);
        return nullFrame;
    }

    ret = avcodec_decode_video2(m_dec_ctx, m_frame, &got_frame, &m_packet);
    if(ret < 0)
    {
        av_free_packet(&m_packet);
        return nullFrame;
    }

    if(got_frame == 0)
    {
        av_free_packet(&m_packet);
        return nullFrame;
    }

    if(m_frameData[0] == NULL)
    {
        for(int i = 0; i < 3; i ++)
            m_frameData[i] = (unsigned char*)malloc(m_dec_ctx->height * m_dec_ctx->width * 1.5);
    }

    int offU = m_dec_ctx->height * m_dec_ctx->width;
    int offV = (int)((m_dec_ctx->height * m_dec_ctx->width) * 1.25f);
    for(int i = 0; i < m_dec_ctx->height; i ++)
        memcpy(&m_frameData[m_index][i * m_dec_ctx->width], &m_frame->data[0][m_frame->linesize[0] * i], m_dec_ctx->width);
    for(int i = 0; i < m_dec_ctx->height / 2; i ++)
    {
        memcpy(&m_frameData[m_index][offU + i * (m_dec_ctx->width / 2)], &m_frame->data[1][m_frame->linesize[1] * i], m_dec_ctx->width / 2);
        memcpy(&m_frameData[m_index][offV + i * (m_dec_ctx->width / 2)], &m_frame->data[2][m_frame->linesize[2] * i], m_dec_ctx->width / 2);
    }

    YUVFrame retFrame;
    retFrame.yuvData = m_frameData[m_index];
    retFrame.width = m_dec_ctx->width;
    retFrame.height = m_dec_ctx->height;
    retFrame.frameTime = frameTime;

    m_index ++;
    if(m_index == 3)
        m_index = 0;

    if(m_frame->key_frame == 1)
        m_receiveIFrame ++;

    av_frame_unref(m_frame);
    av_free_packet(&m_packet);

    if(m_receiveIFrame < 3)
        return nullFrame;

    return retFrame;
}

void IpCamera::run()
{
    AVCodec *dec;
    int ret;

    if(m_streamUrl.isEmpty())
        return;

    AVDictionary *d = NULL;
    ret = av_dict_set(&d, "rtsp_transport", "tcp", 0);
    ret = av_dict_set(&d, "stimeout", "5000000", 0);

    if ((ret = avformat_open_input(&m_fmt_ctx, m_streamUrl.toUtf8().data(), NULL, &d)) < 0)
    {
        if(m_reconnect)
            emit requestReconnect();
        return;
    }

    if ((ret = avformat_find_stream_info(m_fmt_ctx, NULL)) < 0)
    {
        avformat_close_input(&m_fmt_ctx);
        if(m_reconnect)
            emit requestReconnect();
        return;
    }

    /* select the video stream */
    ret = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0)
    {
        avformat_close_input(&m_fmt_ctx);
        if(m_reconnect)
            emit requestReconnect();
        return;
    }

    m_video_stream_index = ret;
    m_dec_ctx = m_fmt_ctx->streams[m_video_stream_index]->codec;
    av_opt_set_int(m_dec_ctx, "refcounted_frames", 1, 0);

    /* init the video decoder */
    if ((ret = avcodec_open2(m_dec_ctx, dec, NULL)) < 0) {
        avcodec_close(m_dec_ctx);
        avformat_close_input(&m_fmt_ctx);
        m_dec_ctx = NULL;
        if(m_reconnect)
            emit requestReconnect();
        return;
    }

    AVStream* videoStream = m_fmt_ctx->streams[m_video_stream_index];
    if(videoStream->r_frame_rate.den == 0)
        m_frameDelay = 1;
    else
    {
        double frameDelay = av_q2d(videoStream->r_frame_rate);
        m_frameDelay = 1000 * frameDelay;
    }

    m_frame = av_frame_alloc();
    av_init_packet(&m_packet);

    QTime receiveTime;

    m_frameMutex.lock();
    memset(&m_curFrame, 0, sizeof(m_curFrame));
    m_frameMutex.unlock();

    m_frameIndex = 0;
    setRunning(1);
    while(isRunning())
    {
        YUVFrame yuvFrame = readFrame();
        if(yuvFrame.yuvData == NULL)
        {
            if(receiveTime.msecsTo(QTime::currentTime()) > TIME_OUT)
                break;

            QThread::msleep(1);
            continue;
        }

        m_frameMutex.lock();
        m_curFrame.width = yuvFrame.width;
        m_curFrame.height = yuvFrame.height;
        m_curFrame.frameTime = yuvFrame.frameTime;
        if(m_curFrame.yuvData == NULL)
            m_curFrame.yuvData = (unsigned char*)malloc(m_curFrame.width * m_curFrame.height * 3 / 2);

        memcpy(m_curFrame.yuvData, yuvFrame.yuvData, m_curFrame.width * m_curFrame.height * 3 / 2);

        m_frameMutex.unlock();
        emit frameChanged();

        receiveTime = QTime::currentTime();
        QThread::msleep(m_frameDelay);

        m_frameIndex ++;
        framePositionChanged(m_fmt_ctx->duration / 1000, (int64_t)m_frameIndex * m_frameDelay);
    }

    m_frameMutex.lock();
    if(m_curFrame.yuvData)
        free(m_curFrame.yuvData);
    memset(&m_curFrame, 0, sizeof(m_curFrame));
    m_frameMutex.unlock();

    avcodec_close(m_dec_ctx);
    avformat_close_input(&m_fmt_ctx);
    av_frame_free(&m_frame);
    m_dec_ctx = NULL;

    if(m_frameData[0])
    {
        for(int i = 0; i < 3; i ++)
            free(m_frameData[i]);

        memset(m_frameData, 0, sizeof(m_frameData));
    }
    m_receiveIFrame = 0;

    setRunning(0);

    if(m_reconnect)
    {
        QThread::msleep(500);
        emit requestReconnect();
    }
}
