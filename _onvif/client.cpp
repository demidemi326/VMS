#include "client.h"
#include <QEventLoop>
#include <QUrl>
#include <QDebug>

using namespace ONVIF;

Client::Client(const QString &url) {
    mUrl = url;

    m_accessManager = new QNetworkAccessManager;
    connect(m_accessManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
    connect(m_accessManager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(m_accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(accessFinished(QNetworkReply*)));
}

QString Client::sendData(const QString &data) {

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setUrl(QUrl(mUrl));

    QNetworkReply* reply = m_accessManager->post(request, data.toUtf8());
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));

    m_loop.exec();

    return reply->readAll();
}


void Client::accessFinished(QNetworkReply* reply)
{
    m_loop.quit();
}

void Client::replyError(QNetworkReply::NetworkError)
{
    m_loop.quit();
}

void Client::readyRead()
{
    m_loop.quit();
}

void Client::sslErrors(QNetworkReply*,QList<QSslError>)
{
    m_loop.quit();
}

void Client::networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessibility)
{
    m_loop.quit();
}
