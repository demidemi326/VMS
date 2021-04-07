#include "message.h"
#include <QUuid>
#include <QTime>
#include <QDateTime>
#include <QStringList>
#include <QCryptographicHash>
#include <QDebug>

using namespace ONVIF;


QDomElement ONVIF::hashToXml(const QString &name,const QHash<QString, QString> &hash) {
    QDomElement element = newElement(name);
    QHashIterator<QString, QString> i(hash);
    while(i.hasNext()) {
        i.next();
        element.appendChild(newElement(i.key(), i.value()));
    }
    return element;
}

QDomElement ONVIF::listToXml(const QString &name, const QString &itemName, const QStringList &list) {
    QDomElement element = newElement(name);
    for(int i = 0; i < list.length(); i++) {
        element.appendChild(newElement(itemName, list.at(i)));
    }
    return element;
}

QDomElement ONVIF::newElement(const QString &name, const QString &value) {
    QDomDocument doc;
    QDomElement element = doc.createElement(name);
    if(value != "") {
        QDomText textNode = doc.createTextNode(value);
        element.appendChild(textNode);
    }
    doc.appendChild(element);
    return doc.firstChildElement();
}


Message *Message::getOnvifSearchMessage() {
    QHash<QString, QString> namespaces;
    namespaces.insert("a", "http://schemas.xmlsoap.org/ws/2004/08/addressing");
    namespaces.insert("d", "http://schemas.xmlsoap.org/ws/2005/04/discovery");
    namespaces.insert("dn", "http://www.onvif.org/ver10/network/wsdl");
    namespaces.insert("dn", "http://www.onvif.org/ver10/network/wsdl");
    Message *msg = new Message(namespaces);
    QDomElement action = newElement("a:Action", "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe");
    QDomElement message_id = newElement("a:MessageID", "uuid:" + msg->uuid());
    QDomElement to = newElement("a:To", "urn:schemas-xmlsoap-org:ws:2005:04:discovery");
    msg->appendToHeader(action);
    msg->appendToHeader(message_id);
    msg->appendToHeader(to);
    
    QDomElement probe = newElement("d:Probe");
    probe.appendChild(newElement("d:Types", "dn:NetworkVideoTransmitter"));
    probe.appendChild(newElement("d:Scopes"));
    msg->appendToBody(probe);
    
    return msg;
}


Message* Message::getMessageWithUserInfo(QHash<QString, QString> &namespaces, const QString &name, const QString &passwd) {
    QString passwdStr = passwd;
    QDateTime curDateTime = QDateTime::currentDateTime();

    char raw_nonce[22]={0xea, 0xde, 0x29, 0xa0, 0x3c, 0x39, 0x65, 0x4f, 0x95, 0xe2, 0x15, 0x11, 0x3a, 0xf0, 0x9f, 0x3f, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0};

    QByteArray nonceData = QByteArray(raw_nonce, sizeof(raw_nonce)).toBase64();
    QCryptographicHash cryptHash(QCryptographicHash::Sha1);

    QString dateStr = curDateTime.toTimeSpec(Qt::UTC).toString("yyyy-MM-ddThh:mm:ss.zZ").toUtf8();

    cryptHash.addData(QByteArray(raw_nonce, sizeof(raw_nonce)));
    cryptHash.addData(dateStr.toUtf8());
    cryptHash.addData(passwdStr.toUtf8());
    QString digestStr = cryptHash.result().toBase64();

    Message *msg = new Message(namespaces);
    QDomElement security = newElement("Security");

    QDomElement usernameToken = newElement("UsernameToken");
    QDomElement username = newElement("Username", name);
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
    if(!name.isEmpty() && !passwd.isEmpty())
    msg->appendToHeader(security);
    return msg;
}


Message::Message(const QHash<QString, QString> &namespaces, QObject *parent) : QObject(parent) {
    this->mNamespaces = namespaces;
    mDoc.appendChild(mDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
    mEnv = mDoc.createElementNS("http://www.w3.org/2003/05/soap-envelope", "s:Envelope");
    mHeader = mDoc.createElement("s:Header");
    mBody = mDoc.createElement("s:Body");
    mBody.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    mBody.setAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
}

QString Message::toXmlStr() {
    QHashIterator<QString, QString> i(mNamespaces);
    while (i.hasNext()) {
        i.next();
        mEnv.setAttribute("xmlns:" + i.key(), i.value());
    }
    
    mEnv.appendChild(mHeader);
    mEnv.appendChild(mBody);
    mDoc.appendChild(mEnv);
    return mDoc.toString();
}

QString Message::uuid() {
    QUuid id = QUuid::createUuid();
    return id.toString();
}

void Message::appendToBody(const QDomElement &body) {
    mBody.appendChild(body);
}

void Message::appendToHeader(const QDomElement &header) {
    mHeader.appendChild(header);
}
