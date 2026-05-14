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
#include <QWebSocket>
#include <QProcess>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);

    void sendRequest(const QString &url, const QUrlQuery &query = QUrlQuery());

    // Устанавливает WebSocket-соединение с сервером для получения уведомлений об обновлениях.
    // После подключения клиент ожидает уведомлений; при получении — запускает процесс обновления.
    void connectUpdateSocket(const QString &wsUrl);

    // Проверяет целостность файлов приложения, сравнивая локальные хеши с серверными.
    // При несоответствии определяет проблемный модуль и инициирует его замену.
    void checkIntegrity(const QJsonObject &serverHashes);

    // Завершает процесс обновления, если он ещё запущен после перезапуска приложения.
    // Вызывается при старте, чтобы не оставлять "зависший" апдейтер.
    void terminateUpdaterIfRunning();

private slots:
    void onRequestFinished(QNetworkReply *reply);

    // Вызывается при получении уведомления об обновлении через WebSocket.
    void onUpdateNotificationReceived(const QString &message);

    // Вызывается при разрыве WebSocket-соединения — пробуем переподключиться.
    void onSocketDisconnected();

    // Вызывается при завершении скачивания файла обновления с сервера.
    void onUpdateDownloadFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QWebSocket            *updateSocket;
    QString                wsUrl;

    // Скачивает зашифрованный файл обновления для указанного модуля.
    // Передаёт публичный RSA-ключ клиента в заголовке для гибридного шифрования.
    void downloadUpdate(const QString &module, const QString &version);

    // Запускает процесс обновления (updater), передавая ему инструкции по установке.
    // После запуска updater завершает текущий процесс, применяет обновление и перезапускает приложение.
    void launchUpdater(const QString &module, const QString &updateFilePath, const QString &expectedHash);

    // Вычисляет SHA-256 хеш указанного файла.
    QString computeFileHash(const QString &filePath);

    // Определяет, какой именно модуль повреждён, сравнивая хеши файлов поштучно.
    // Возвращает имя первого модуля с несовпадающим хешем.
    QString findDamagedModule(const QJsonObject &serverHashes);

    // Читает публичный RSA-ключ клиента из файла для передачи серверу при скачивании обновления.
    QByteArray loadPublicKey();
};

#endif // HTTPCLIENT_H
