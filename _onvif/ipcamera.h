#ifndef IPCAMERA_H
#define IPCAMERA_H

#include <QtWidgets>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>

#include "devicemanagement.h"
#include "mediamanagement.h"

using namespace ONVIF;

typedef struct _tagYUVFrame
{
    unsigned char*  yuvData;
    int         width;
    int         height;
    qint64      frameTime;
}YUVFrame;

class IpCamera : public QThread
{
    Q_OBJECT
public:
    explicit IpCamera(QObject *parent = 0);
    ~IpCamera();

    static void    init();

    bool    open(QString streamUri);
    void    close();

    void    setReconnect(int reconnect);

    int     isRunning();
    void    setRunning(int);

    void    getCurFrame(QByteArray& frameData, int& width, int& height, qint64& imageTime);

signals:
    void    requestReconnect();
    void    frameChanged();
    void    framePositionChanged(qint64 duration, qint64 position);
    void    logOut(QString);

protected:
    void    run();
    YUVFrame    readFrame();

protected:
    AVFormatContext *m_fmt_ctx;
    AVCodecContext *m_dec_ctx;
    int m_video_stream_index;

    AVPacket m_packet;
    AVFrame *m_frame;

    QMutex              m_mutex;
    int                 m_running;

    unsigned char*      m_frameData[3];
    int                 m_index;

    MediaManagement*    m_mdediaManagement;
    QString             m_ipAddress;
    QString             m_userName;
    QString             m_password;

    QString             m_streamUrl;

    int                 m_sender;
    int                 m_receiveIFrame;

    int                 m_reconnect;

    double              m_frameDelay;

    YUVFrame            m_curFrame;
    QMutex              m_frameMutex;
    QWaitCondition      m_frameCond;

    int64_t             m_frameIndex;
};

#endif // IPCAMERA_H
