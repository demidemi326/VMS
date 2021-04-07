#ifndef ONVIF_CLIENT_H
#define ONVIF_CLIENT_H

#include <QObject>
#include <QtNetwork>

namespace ONVIF {
    class Client : public QObject {
        Q_OBJECT
    public:
        explicit Client(const QString &url);

        QString sendData(const QString &data);
    private:

    private slots:
        void accessFinished(QNetworkReply* reply);
        void replyError(QNetworkReply::NetworkError);
        void readyRead();
        void sslErrors(QNetworkReply*,QList<QSslError>);
        void networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility);
    private:
        QString mUrl;
        QNetworkAccessManager* m_accessManager;
        QEventLoop m_loop;
        QNetworkReply*  m_reply;

        int m_sending;
    };
}

#endif // ONVIF_CLIENT_H
