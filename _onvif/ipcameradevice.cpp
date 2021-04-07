#include "ipcameradevice.h"
#include "devicemanagement.h"
#include "mediamanagement.h"
#include "eventmanagement.h"

#include <QtWidgets>

using namespace ONVIF;

IpCameraDevice::IpCameraDevice(QObject *parent) : QObject(parent)
{

}

IpCameraDevice::~IpCameraDevice()
{

}


void IpCameraDevice::setDeviceService(QString serviceStr, QString ipAddressStr, QString userNameStr, QString passwordStr)
{
    m_deviceServiceStr = serviceStr;
    m_ipAddressStr = ipAddressStr;
    m_userNameStr = userNameStr;
    m_passwordStr = passwordStr;

    slotStartFind();
    //QTimer::singleShot(10, this, SLOT(slotStartFind()));
}

QString IpCameraDevice::deviceService()
{
    return m_deviceServiceStr;
}

void IpCameraDevice::slotStartFind()
{
    qDebug() << "start find";
    DeviceManagement* deviceManagement = new DeviceManagement(m_deviceServiceStr, m_userNameStr, m_passwordStr);
    QHash<QString, QString> deviceInfos = deviceManagement->getDeviceInformation();
    if(!deviceInfos.size())
    {
        delete deviceManagement;
        return;
    }

    QHash<QString, QString> deviceServices = deviceManagement->getServices();
    if(!deviceServices.size())
    {
        delete deviceManagement;
        return;
    }

    if(!m_userNameStr.isEmpty() || !m_passwordStr.isEmpty())
    {
        EventManagement* eventManagement = new EventManagement(deviceServices["Event"], m_userNameStr, m_passwordStr);
        QString pullPointStr = eventManagement->createPullPointSubscription();

//        EventManagement* pullPointManagement = new EventManagement(pullPointStr, m_userNameStr, m_passwordStr);
//        pullPointManagement->pullMessages();

//        delete pullPointManagement;
        delete eventManagement;
    }

    MediaManagement* mediaManagment = new MediaManagement(deviceServices["Media"], m_userNameStr, m_passwordStr);
    Profiles* profile = mediaManagment->getProfiles();
    if(profile == NULL || !profile->m_toKenPro.size())
    {
        delete deviceManagement;
        delete mediaManagment;
        return;
    }

    QString oldSourceName;
    for(int i = 0; i < profile->m_toKenPro.size(); i ++)
    {
        if(oldSourceName != profile->m_sourceTokenVsc[i])
        {
            QString profileToken = profile->m_toKenPro[i];
            StreamUri* streamUri = mediaManagment->getStreamUri(profileToken);
            if(streamUri == NULL)
                continue;

            QString streamUrl = streamUri->uri();
            streamUrl.remove("amp;");

            if(streamUrl.isEmpty())
                continue;

            QString userNameAndPass;
            userNameAndPass.sprintf("%s:%s@", m_userNameStr.toUtf8().data(), m_passwordStr.toUtf8().data());

            QStringList splitUri = streamUrl.split("/");
            if(splitUri.size() < 4)
                return;

            streamUrl = "";
            for(int i = 0; i < splitUri.size(); i ++)
            {
                if(i == 2)
                {
                    streamUrl += userNameAndPass;
                    QStringList splitUriIp = splitUri[i].split(":");
                    if(splitUriIp.size() == 2)
                        streamUrl += (m_ipAddressStr + ":" + splitUriIp[1]);
                    else
                        streamUrl += m_ipAddressStr;
                }
                else
                    streamUrl += splitUri[i];

                if(i != splitUri.size() - 1)
                    streamUrl.append("/");
            }

            QString mediaService = deviceServices["Media"];
            QStringList serviceList = mediaService.split("/");
            if(serviceList.size() < 2)
                continue;

            deviceInfos.insert("videosource", profile->m_sourceTokenVsc[i]);
            deviceInfos.insert("username", m_userNameStr);
            deviceInfos.insert("password", m_passwordStr);
            deviceInfos.insert("ip", serviceList[2]);
            deviceInfos.insert("streamuri", streamUrl);

            findNewCamera(deviceInfos);
            //qDebug() << "find new camera" << deviceServices << profile->m_sourceTokenVsc[i] << streamUrl;

            oldSourceName = profile->m_sourceTokenVsc[i];
        }
    }

    delete deviceManagement;
    delete mediaManagment;
}
