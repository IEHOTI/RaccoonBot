#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QJsonObject>

class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);
    void sendRequest(const QString &url, const QUrlQuery &query = QUrlQuery());

private slots:
    void onRequestFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
};

#endif // HTTPCLIENT_H

