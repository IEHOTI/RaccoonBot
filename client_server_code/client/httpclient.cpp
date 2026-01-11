#include "httpclient.h"

HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);

    // Подключаем сигнал завершения запроса
    connect(manager, &QNetworkAccessManager::finished,
            this, &HttpClient::onRequestFinished);
}

void HttpClient::sendRequest(const QString &url, const QUrlQuery &query)
{
    QUrl requestUrl(url);

    if (!query.isEmpty()) {
        requestUrl.setQuery(query);
    }

    QNetworkRequest request(requestUrl);

    qDebug() << "Отправка запроса на:" << requestUrl.toString();

    // Отправляем GET запрос
    //QNetworkReply *reply =
    manager->get(request);
}

void HttpClient::onRequestFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        // Читаем ответ
        QByteArray responseData = reply->readAll();

        qDebug() << "Запрос выполнен успешно!";
        qDebug() << "Статус код:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "Полученные данные:";
        qDebug() << "==========================================";

        // Пытаемся распарсить JSON для красивого вывода
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

        int id;
        QString subscribe,duration;
        QJsonObject jsonObj = jsonDoc.object();
        id = jsonObj["id"].toInt();
        subscribe = jsonObj["subscribe"].toString();
        duration = jsonObj["duration"].toString();

        // Парсим строку в QDateTime
        QDateTime dateTime = QDateTime::fromString(duration, Qt::ISODate);

        if (parseError.error == QJsonParseError::NoError && !jsonDoc.isNull()) {
            qDebug().noquote() << "id:" << id << subscribe << duration << dateTime.date() << dateTime.time();
        } else {
            // Если не JSON, выводим как обычный текст
            qDebug().noquote() << responseData;
        }

        qDebug() << "==========================================";
        fflush(stdout);  // для stdout
        fflush(stderr);  // для stderr

    } else {
        qDebug() << "Ошибка запроса:" << reply->errorString();
        qDebug() << "Код ошибки:" << reply->error();

        // Выводим тело ответа даже при ошибке (может содержать полезную информацию)
        QByteArray responseData = reply->readAll();
        if (!responseData.isEmpty()) {
            qDebug() << "Тело ответа при ошибке:";
            qDebug() << responseData;
        }
        fflush(stdout);  // для stdout
        fflush(stderr);  // для stderr
    }

    reply->deleteLater();

    // Завершаем приложение
    QCoreApplication::quit();
}
