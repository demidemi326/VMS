#ifndef EVENTMANAGEMENT_H
#define EVENTMANAGEMENT_H

#include "service.h"
#include <QDateTime>
#include "device_management/systemdateandtime.h"
#include "device_management/user.h"
#include "device_management/systemfactorydefault.h"
#include "device_management/systemreboot.h"
#include "device_management/networkinterfaces.h"
#include "device_management/networkprotocols.h"
#include "device_management/capabilities.h"

namespace ONVIF {
    class EventManagement : public Service
    {
        Q_OBJECT
    public:
        explicit EventManagement(const QString & wsdlUrl, const QString &username, const QString &password);
        ~EventManagement();

        QString    createPullPointSubscription();

        void       pullMessages();

    protected:
        Message *newMessage();
        Message *newMessageActionPullMessage();
        QHash<QString, QString> namespaces(const QString &key);

        QString m_wsdlUrl;
        QString m_userName;
        QString m_password;
    };

}
#endif // EVENTMANAGEMENT_H
