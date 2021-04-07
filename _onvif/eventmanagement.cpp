#include "eventmanagement.h"

#include <QUuid>

using namespace ONVIF;

EventManagement::EventManagement(const QString & wsdlUrl, const QString &username, const QString &password)
    :Service(wsdlUrl, username, password)
{
    m_wsdlUrl = wsdlUrl;
    m_userName = username;
    m_password = password;
}

EventManagement::~EventManagement()
{

}


QHash<QString, QString> EventManagement::namespaces(const QString &key)
{
    QHash<QString, QString> names;
    names.insert("SOAP-ENV", "http://www.w3.org/2003/05/soap-envelope");
    names.insert("SOAP-ENC", "http://www.w3.org/2003/05/soap-encoding");
    names.insert("xsi", "http://www.w3.org/2001/XMLSchema-instance");
    names.insert("xsd", "http://www.w3.org/2001/XMLSchema");
    names.insert("c14n", "http://www.w3.org/2001/10/xml-exc-c14n#");
    names.insert("wsu", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd");
    names.insert("xenc", "http://www.w3.org/2001/04/xmlenc#");
    names.insert("ds", "http://www.w3.org/2000/09/xmldsig#");
    names.insert("wsse", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd");
    names.insert("wsa5", "http://www.w3.org/2005/08/addressing");
    names.insert("xmime", "http://tempuri.org/xmime.xsd");
    names.insert("xop", "http://www.w3.org/2004/08/xop/include");
    names.insert("wsa", "http://schemas.xmlsoap.org/ws/2004/08/addressing");
    names.insert("tt", "http://www.onvif.org/ver10/schema");
    names.insert("wsbf", "http://docs.oasis-open.org/wsrf/bf-2");
    names.insert("wstop", "http://docs.oasis-open.org/wsn/t-1");
    names.insert("d", "http://schemas.xmlsoap.org/ws/2005/04/discovery");
    names.insert("wsr", "http://docs.oasis-open.org/wsrf/r-2");
    names.insert("dndl", "http://www.onvif.org/ver10/network/wsdl/DiscoveryLookupBinding");
    names.insert("dnrd", "http://www.onvif.org/ver10/network/wsdl/RemoteDiscoveryBinding");
    names.insert("dn", "http://www.onvif.org/ver10/network/wsdl");
    names.insert("tad", "http://www.onvif.org/ver10/analyticsdevice/wsdl");
    names.insert("tanae", "http://www.onvif.org/ver20/analytics/wsdl/AnalyticsEngineBinding");
    names.insert("tanre", "http://www.onvif.org/ver20/analytics/wsdl/RuleEngineBinding");
    names.insert("tan", "http://www.onvif.org/ver20/analytics/wsdl");
    names.insert("tds", "http://www.onvif.org/ver10/device/wsdl");
    names.insert("tetcp", "http://www.onvif.org/ver10/events/wsdl/CreatePullPointBinding");
    names.insert("tete", "http://www.onvif.org/ver10/events/wsdl/EventBinding");
    names.insert("tetnc", "http://www.onvif.org/ver10/events/wsdl/NotificationConsumerBinding");
    names.insert("tetnp", "http://www.onvif.org/ver10/events/wsdl/NotificationProducerBinding");
    names.insert("tetpp", "http://www.onvif.org/ver10/events/wsdl/PullPointBinding");
    names.insert("tetpps", "http://www.onvif.org/ver10/events/wsdl/PullPointSubscriptionBinding");
    names.insert("tev", "http://www.onvif.org/ver10/events/wsdl");
    names.insert("tetps", "http://www.onvif.org/ver10/events/wsdl/PausableSubscriptionManagerBinding");
    names.insert("wsnt", "http://docs.oasis-open.org/wsn/b-2");
    names.insert("tetsm", "http://www.onvif.org/ver10/events/wsdl/SubscriptionManagerBinding");
    names.insert("timg", "http://www.onvif.org/ver20/imaging/wsdl");
    names.insert("timg10", "http://www.onvif.org/ver10/imaging/wsdl");
    names.insert("tls", "http://www.onvif.org/ver10/display/wsdl");
    names.insert("tmd", "http://www.onvif.org/ver10/deviceIO/wsdl");
    names.insert("tptz", "http://www.onvif.org/ver20/ptz/wsdl");
    names.insert("trc", "http://www.onvif.org/ver10/recording/wsdl");
    names.insert("trp", "http://www.onvif.org/ver10/replay/wsdl");
    names.insert("trt", "http://www.onvif.org/ver10/media/wsdl");
    names.insert("trv", "http://www.onvif.org/ver10/receiver/wsdl");
    names.insert("tse", "http://www.onvif.org/ver10/search/wsdl");
    names.insert("tns1", "http://www.onvif.org/ver10/schema");
    names.insert("tnsn", "http://www.eventextension.com/2011/event/topics");
    names.insert("tnsavg", "http://www.avigilon.com/onvif/ver10/topics");

    return names;
}


Message *EventManagement::newMessage()
{
    QHash<QString, QString> names;
    names.insert("a", "http://www.w3.org/2005/08/addressing");

    Message* newMessage = Message::getMessageWithUserInfo(names, m_userName, m_password);

    QDomElement actionEl = newElement("a:Action", "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionRequest");
    actionEl.setAttribute("s:mustUnderstand", "1");

    QString uuidStr = "urn:uuid:" + QUuid::createUuid().toString();
    uuidStr.replace("{", "");
    uuidStr.replace("}", "");

    QDomElement messageIDEl = newElement("a:MessageID", uuidStr);

    QDomElement replyToEl = newElement("a:ReplyTo");
    QDomElement addressEl = newElement("a:Address", "http://www.w3.org/2005/08/addressing/anonymous");
    replyToEl.appendChild(addressEl);

    QDomElement toEl = newElement("a:To", m_wsdlUrl);
    toEl.setAttribute("s:mustUnderstand", "1");

    newMessage->appendToHeader(actionEl);
    newMessage->appendToHeader(messageIDEl);
    newMessage->appendToHeader(replyToEl);
    newMessage->appendToHeader(toEl);

    return newMessage;
}


QString EventManagement::createPullPointSubscription()
{
    Message *msg = newMessage();
    QDomElement pullMessageEl = newElement("CreatePullPointSubscription");
    pullMessageEl.setAttribute("xmlns", "http://www.onvif.org/ver10/events/wsdl");

//    QDomElement timeOutEl = newElement("InitialTerminationTime", "PT600S");
//    pullMessageEl.appendChild(timeOutEl);

    msg->appendToBody(pullMessageEl);

    MessageParser *result = sendMessage(msg);
    QString pullPointStr;
    if(result)
    {
        pullPointStr = result->getValue("//wsa5:Address");
        if(!pullPointStr.isEmpty())
            return pullPointStr;

        pullPointStr = result->getValue("//wsa:Address");
        if(!pullPointStr.isEmpty())
            return pullPointStr;
    }
    return pullPointStr;
}


Message *EventManagement::newMessageActionPullMessage()
{
    QHash<QString, QString> names;
    names.insert("a", "http://www.w3.org/2005/08/addressing");

    QString passwdStr = m_password;
    QDateTime curDateTime = QDateTime::currentDateTime();

    char raw_nonce[22]={0xea, 0xde, 0x29, 0xa0, 0x3c, 0x39, 0x65, 0x4f, 0x95, 0xe2, 0x15, 0x11, 0x3a, 0xf0, 0x9f, 0x3f, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0};

    QByteArray nonceData = QByteArray(raw_nonce, sizeof(raw_nonce)).toBase64();
    QCryptographicHash cryptHash(QCryptographicHash::Sha1);

    QString dateStr = curDateTime.toTimeSpec(Qt::UTC).toString("yyyy-MM-ddThh:mm:ss.zZ").toUtf8();

    cryptHash.addData(QByteArray(raw_nonce, sizeof(raw_nonce)));
    cryptHash.addData(dateStr.toUtf8());
    cryptHash.addData(passwdStr.toUtf8());
    QString digestStr = cryptHash.result().toBase64();

    Message *newMessage = new Message(names);
    QDomElement security = newElement("Security");

    QDomElement usernameToken = newElement("UsernameToken");
    QDomElement username = newElement("Username", m_userName);
    QDomElement password = newElement("Password", digestStr);
    QDomElement nonce = newElement("Nonce", nonceData);
    QDomElement created = newElement("Created", dateStr);

    password.setAttribute("Type", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest");
    nonce.setAttribute("EncodingType", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary");
    created.setAttribute("xmlns", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd");

    usernameToken.appendChild(username);
    usernameToken.appendChild(password);
    usernameToken.appendChild(nonce);
    usernameToken.appendChild(created);

    security.appendChild(usernameToken);
    security.setAttribute("xmlns", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd");
    security.setAttribute("s:mustUnderstand", "1");

    QDomElement actionEl = newElement("a:Action", "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesRequest");
    actionEl.setAttribute("s:mustUnderstand", "1");

    QString uuidStr = "urn:uuid:" + QUuid::createUuid().toString();
    uuidStr.replace("{", "");
    uuidStr.replace("}", "");

    QDomElement messageIDEl = newElement("a:MessageID", uuidStr);

    QDomElement replyToEl = newElement("a:ReplyTo");
    QDomElement addressEl = newElement("a:Address", "http://www.w3.org/2005/08/addressing/anonymous");
    replyToEl.appendChild(addressEl);

    QDomElement toEl = newElement("a:To", m_wsdlUrl);
    toEl.setAttribute("s:mustUnderstand", "1");

    newMessage->appendToHeader(actionEl);
    newMessage->appendToHeader(messageIDEl);
    newMessage->appendToHeader(replyToEl);
    if(!m_userName.isEmpty() && !m_password.isEmpty())
        newMessage->appendToHeader(security);
    newMessage->appendToHeader(toEl);

    return newMessage;
}
void EventManagement::pullMessages()
{
    Message *msg = newMessageActionPullMessage();
    QDomElement pullMessageEl = newElement("PullMessages");
    pullMessageEl.setAttribute("xmlns", "http://www.onvif.org/ver10/events/wsdl");

    QDomElement timeOutEl = newElement("TimeOut", "PT1M");
    QDomElement messageLimitEl = newElement("MessageLimit", "1024");

    pullMessageEl.appendChild(timeOutEl);
    pullMessageEl.appendChild(messageLimitEl);

    msg->appendToBody(pullMessageEl);

    sendMessage(msg);
}
