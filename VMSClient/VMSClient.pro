
#-------------------------------------------------
#
# Project created by QtCreator 2014-12-12T22:21:06
#
#-------------------------------------------------


QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl multimedia xml network xmlpatterns

TARGET = VMSClient_video
TEMPLATE = app

DEFINES +=

DESTDIR = ../_VMS_Product
QMAKE_LIBDIR += ../_VMS_Product

win32:RC_FILE = VMSClient.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    mediaitem.cpp \
    facerequestsocket.cpp \
    serverdlg.cpp \
    ../_base/socketbase.cpp \
    stringtable.cpp \
    serverinfosocket.cpp \
    ipcamerasettingdlg.cpp \
    monitoringareadlg.cpp \
    checktreeview.cpp \
    areaselectdlg.cpp \
    ipcameraonesettingdlg.cpp \
    logviewitem.cpp \
    logselectdlg.cpp \
    sceneimagereceivesocket.cpp \
    addblacklistfromcaptureddlg.cpp \
    imageprocessingsocket.cpp \
    frameresultitem.cpp \
    nextbuttonitem.cpp \
    blackinforeceivesocket.cpp \
    gendereditor.cpp \
    editblackinfodlg.cpp \
    editoneblacklistsocket.cpp \
    deleteblacklistsocket.cpp \
    blackrecogresultreceivesocket.cpp \
    blackresultview.cpp \
    blackresultdetaildlg.cpp \
    blackenrollbatchdlg.cpp \
    imagesearchengine.cpp \
    addblackinfosocket.cpp \
    logresultreceivesocket.cpp \
    blackfaceadddlg.cpp \
    monitoringipcamerasettngdlg.cpp \
    monitoringserveillancesettingdlg.cpp \
    searchlogsocket.cpp \
    searchblacksocket.cpp \
    cachelogviewitem.cpp \
    warningplayer.cpp \
    aboutdlg.cpp \
    searchlastblacksocket.cpp \
    nokeygraphicsview.cpp \
    selectchaneldlg.cpp \
    modifypass.cpp \
    newpass.cpp \
    removepass.cpp \
    logindlg.cpp \
    resultalarmdlg.cpp \
    ../_onvif/device_management/capabilities.cpp \
    ../_onvif/device_management/networkinterfaces.cpp \
    ../_onvif/device_management/networkprotocols.cpp \
    ../_onvif/device_management/systemdateandtime.cpp \
    ../_onvif/device_management/systemfactorydefault.cpp \
    ../_onvif/device_management/systemreboot.cpp \
    ../_onvif/device_management/user.cpp \
    ../_onvif/media_management/audioencoderconfiguration.cpp \
    ../_onvif/media_management/audioencoderconfigurationoptions.cpp \
    ../_onvif/media_management/audioencoderconfigurations.cpp \
    ../_onvif/media_management/audiosourceconfiigurations.cpp \
    ../_onvif/media_management/profile.cpp \
    ../_onvif/media_management/profiles.cpp \
    ../_onvif/media_management/streamuri.cpp \
    ../_onvif/media_management/videoencoderconfiguration.cpp \
    ../_onvif/media_management/videoencoderconfigurationoptions.cpp \
    ../_onvif/media_management/videoencoderconfigurations.cpp \
    ../_onvif/media_management/videosourceconfiguration.cpp \
    ../_onvif/media_management/videosourceconfigurations.cpp \
    ../_onvif/ptz_management/absolutemove.cpp \
    ../_onvif/ptz_management/configuration.cpp \
    ../_onvif/ptz_management/configurations.cpp \
    ../_onvif/ptz_management/continuousmove.cpp \
    ../_onvif/ptz_management/gotohomeposition.cpp \
    ../_onvif/ptz_management/gotopreset.cpp \
    ../_onvif/ptz_management/node.cpp \
    ../_onvif/ptz_management/nodes.cpp \
    ../_onvif/ptz_management/preset.cpp \
    ../_onvif/ptz_management/presets.cpp \
    ../_onvif/ptz_management/relativemove.cpp \
    ../_onvif/ptz_management/removepreset.cpp \
    ../_onvif/ptz_management/stop.cpp \
    ../_onvif/client.cpp \
    ../_onvif/devicemanagement.cpp \
    ../_onvif/devicesearcher.cpp \
    ../_onvif/ipcamera.cpp \
    ../_onvif/mediamanagement.cpp \
    ../_onvif/message.cpp \
    ../_onvif/messageparser.cpp \
    ../_onvif/ptzmanagement.cpp \
    ../_onvif/service.cpp \
    camerasurfaceglitem.cpp \
    camerasurfaceglview.cpp \
    controlbutton.cpp \
    selectcameradlg.cpp \
    ../_onvif/eventmanagement.cpp \
    ../_onvif/ipcameradevice.cpp \
    searchcameraitem.cpp \
    camerasurface.cpp \
    clientbase.cpp \
    ../_base/base.cpp \
    camerasurfaceview.cpp

HEADERS  += mainwindow.h \
    mediaitem.h \
    facerequestsocket.h \
    serverdlg.h \
    enginestruct.h \
    ../_base/socketbase.h \
    stringtable.h \
    serverinfosocket.h \
    ipcamerasettingdlg.h \
    monitoringareadlg.h \
    checktreeview.h \
    areaselectdlg.h \
    ipcameraonesettingdlg.h \
    logviewitem.h \
    logselectdlg.h \
    sceneimagereceivesocket.h \
    addblacklistfromcaptureddlg.h \
    imageprocessingsocket.h \
    frameresultitem.h \
    nextbuttonitem.h \
    blackinforeceivesocket.h \
    gendereditor.h \
    editblackinfodlg.h \
    editoneblacklistsocket.h \
    deleteblacklistsocket.h \
    blackrecogresultreceivesocket.h \
    blackresultview.h \
    blackresultdetaildlg.h \
    blackenrollbatchdlg.h \
    imagesearchengine.h \
    addblackinfosocket.h \
    logresultreceivesocket.h \
    blackfaceadddlg.h \
    monitoringipcamerasettngdlg.h \
    monitoringserveillancesettingdlg.h \
    searchlogsocket.h \
    searchblacksocket.h \
    cachelogviewitem.h \
    warningplayer.h \
    aboutdlg.h \
    searchlastblacksocket.h \
    nokeygraphicsview.h \
    selectchaneldlg.h \
    modifypass.h \
    newpass.h \
    removepass.h \
    logindlg.h \
    resultalarmdlg.h \
    ../include/libavcodec/avcodec.h \
    ../include/libavcodec/avfft.h \
    ../include/libavcodec/dxva2.h \
    ../include/libavcodec/old_codec_ids.h \
    ../include/libavcodec/vaapi.h \
    ../include/libavcodec/vda.h \
    ../include/libavcodec/vdpau.h \
    ../include/libavcodec/version.h \
    ../include/libavcodec/xvmc.h \
    ../include/libavdevice/avdevice.h \
    ../include/libavdevice/version.h \
    ../include/libavfilter/asrc_abuffer.h \
    ../include/libavfilter/avcodec.h \
    ../include/libavfilter/avfilter.h \
    ../include/libavfilter/avfiltergraph.h \
    ../include/libavfilter/buffersink.h \
    ../include/libavfilter/buffersrc.h \
    ../include/libavfilter/version.h \
    ../include/libavformat/avformat.h \
    ../include/libavformat/avio.h \
    ../include/libavformat/version.h \
    ../include/libavutil/adler32.h \
    ../include/libavutil/aes.h \
    ../include/libavutil/attributes.h \
    ../include/libavutil/audio_fifo.h \
    ../include/libavutil/audioconvert.h \
    ../include/libavutil/avassert.h \
    ../include/libavutil/avconfig.h \
    ../include/libavutil/avstring.h \
    ../include/libavutil/avutil.h \
    ../include/libavutil/base64.h \
    ../include/libavutil/blowfish.h \
    ../include/libavutil/bprint.h \
    ../include/libavutil/bswap.h \
    ../include/libavutil/buffer.h \
    ../include/libavutil/channel_layout.h \
    ../include/libavutil/common.h \
    ../include/libavutil/cpu.h \
    ../include/libavutil/crc.h \
    ../include/libavutil/dict.h \
    ../include/libavutil/error.h \
    ../include/libavutil/eval.h \
    ../include/libavutil/ffversion.h \
    ../include/libavutil/fifo.h \
    ../include/libavutil/file.h \
    ../include/libavutil/frame.h \
    ../include/libavutil/hmac.h \
    ../include/libavutil/imgutils.h \
    ../include/libavutil/intfloat.h \
    ../include/libavutil/intfloat_readwrite.h \
    ../include/libavutil/intreadwrite.h \
    ../include/libavutil/lfg.h \
    ../include/libavutil/log.h \
    ../include/libavutil/lzo.h \
    ../include/libavutil/macros.h \
    ../include/libavutil/mathematics.h \
    ../include/libavutil/md5.h \
    ../include/libavutil/mem.h \
    ../include/libavutil/murmur3.h \
    ../include/libavutil/old_pix_fmts.h \
    ../include/libavutil/opt.h \
    ../include/libavutil/parseutils.h \
    ../include/libavutil/pixdesc.h \
    ../include/libavutil/pixfmt.h \
    ../include/libavutil/random_seed.h \
    ../include/libavutil/rational.h \
    ../include/libavutil/ripemd.h \
    ../include/libavutil/samplefmt.h \
    ../include/libavutil/sha.h \
    ../include/libavutil/sha512.h \
    ../include/libavutil/stereo3d.h \
    ../include/libavutil/time.h \
    ../include/libavutil/timecode.h \
    ../include/libavutil/timestamp.h \
    ../include/libavutil/version.h \
    ../include/libavutil/xtea.h \
    ../include/libpostproc/postprocess.h \
    ../include/libpostproc/version.h \
    ../include/libswresample/swresample.h \
    ../include/libswresample/version.h \
    ../include/libswscale/swscale.h \
    ../include/libswscale/version.h \
    ../include/frengine.h \
    ../_onvif/device_management/capabilities.h \
    ../_onvif/device_management/networkinterfaces.h \
    ../_onvif/device_management/networkprotocols.h \
    ../_onvif/device_management/systemdateandtime.h \
    ../_onvif/device_management/systemfactorydefault.h \
    ../_onvif/device_management/systemreboot.h \
    ../_onvif/device_management/user.h \
    ../_onvif/media_management/audioencoderconfiguration.h \
    ../_onvif/media_management/audioencoderconfigurationoptions.h \
    ../_onvif/media_management/audioencoderconfigurations.h \
    ../_onvif/media_management/audiosourceconfigurations.h \
    ../_onvif/media_management/profile.h \
    ../_onvif/media_management/profiles.h \
    ../_onvif/media_management/streamuri.h \
    ../_onvif/media_management/videoencoderconfiguration.h \
    ../_onvif/media_management/videoencoderconfigurationoptions.h \
    ../_onvif/media_management/videoencoderconfigurations.h \
    ../_onvif/media_management/videosourceconfiguration.h \
    ../_onvif/media_management/videosourceconfigurations.h \
    ../_onvif/ptz_management/absolutemove.h \
    ../_onvif/ptz_management/configuration.h \
    ../_onvif/ptz_management/configurations.h \
    ../_onvif/ptz_management/continuousmove.h \
    ../_onvif/ptz_management/gotohomeposition.h \
    ../_onvif/ptz_management/gotopreset.h \
    ../_onvif/ptz_management/node.h \
    ../_onvif/ptz_management/nodes.h \
    ../_onvif/ptz_management/preset.h \
    ../_onvif/ptz_management/presets.h \
    ../_onvif/ptz_management/relativemove.h \
    ../_onvif/ptz_management/removepreset.h \
    ../_onvif/ptz_management/stop.h \
    ../_onvif/client.h \
    ../_onvif/devicemanagement.h \
    ../_onvif/devicesearcher.h \
    ../_onvif/ipcamera.h \
    ../_onvif/mediamanagement.h \
    ../_onvif/message.h \
    ../_onvif/messageparser.h \
    ../_onvif/ptzmanagement.h \
    ../_onvif/service.h \
    camerasurfaceglitem.h \
    camerasurfaceglview.h \
    controlbutton.h \
    selectcameradlg.h \
    ../_onvif/eventmanagement.h \
    ../_onvif/ipcameradevice.h \
    searchcameraitem.h \
    camerasurface.h \
    clientbase.h \
    ../_base/base.h \
    camerasurfaceview.h

INCLUDEPATH += ../include ../_onvif ../_base

LIBS += -lws2_32

FORMS    += mainwindow.ui \
    serverdlg.ui \
    ipcamerasettingdlg.ui \
    monitoringareadlg.ui \
    areaselectdlg.ui \
    ipcameraonesettingdlg.ui \
    logselectdlg.ui \
    addblacklistfromcaptureddlg.ui \
    editblackinfodlg.ui \
    blackresultview.ui \
    blackresultdetaildlg.ui \
    blackenrollbatchdlg.ui \
    blackfaceadddlg.ui \
    monitoringipcamerasettngdlg.ui \
    monitoringserveillancesettingdlg.ui \
    aboutdlg.ui \
    selectchaneldlg.ui \
    modifypass.ui \
    newpass.ui \
    removepass.ui \
    logindlg.ui \
    resultalarmdlg.ui \
    selectcameradlg.ui

RESOURCES += \
    mainResource.qrc


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lavcodec
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lavcodec
else:unix: LIBS += -L$$PWD/../lib/ -lavcodec

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lavdevice
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lavdevice
else:unix: LIBS += -L$$PWD/../lib/ -lavdevice

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lavfilter
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lavfilter
else:unix: LIBS += -L$$PWD/../lib/ -lavfilter

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lavformat
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lavformat
else:unix: LIBS += -L$$PWD/../lib/ -lavformat

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lavutil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lavutil
else:unix: LIBS += -L$$PWD/../lib/ -lavutil

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lpostproc
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lpostproc
else:unix: LIBS += -L$$PWD/../lib/ -lpostproc

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lswresample
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lswresample
else:unix: LIBS += -L$$PWD/../lib/ -lswresample

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -lswscale
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -lswscale
else:unix: LIBS += -L$$PWD/../lib/ -lswscale

INCLUDEPATH += $$PWD/../include
DEPENDPATH += $$PWD/../include
