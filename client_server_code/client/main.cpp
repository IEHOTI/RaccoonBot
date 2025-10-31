#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "httpclient.h"
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    HttpClient client;

    QString baseUrl = "http://192.168.91.201:8080/users";
    for(int i = 100; i < 110; i++) {
        QUrlQuery query;
        query.addQueryItem("id", QString::number(i));
        // Отправляем запрос
        client.sendRequest(baseUrl, query);
        QThread::msleep(250); // 100ms задержка
    }


    // Таймер для безопасности - если что-то пойдет не так
    QTimer::singleShot(10000, &app, &QCoreApplication::quit);

    return app.exec();
}
