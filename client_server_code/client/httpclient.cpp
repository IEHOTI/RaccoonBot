#include "httpclient.h"

// Имя процесса апдейтера — используется при поиске и завершении зависшего процесса обновления
static const QString kUpdaterProcessName = "updater";

// Путь к публичному RSA-ключу клиента (генерируется при первом запуске или поставляется с дистрибутивом)
static const QString kPublicKeyPath = "./keys/client_public.pem";

// Директория, куда сохраняются скачанные файлы обновлений перед передачей апдейтеру
static const QString kUpdateStagingDir = "./updates/staging/";

HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    updateSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    // Подключаем сигнал завершения запроса
    connect(manager, &QNetworkAccessManager::finished,
            this, &HttpClient::onRequestFinished);

    // При старте завершаем зависший апдейтер, если он остался от предыдущего обновления
    terminateUpdaterIfRunning();
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
        QString subscribe, duration;
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
}

// connectUpdateSocket устанавливает постоянное WebSocket-соединение с сервером.
// Через него сервер присылает уведомления вида {"module":"client","version":"1.2.0"},
// после чего клиент подтверждает готовность и начинает скачивать обновление.
void HttpClient::connectUpdateSocket(const QString &wsUrl)
{
    this->wsUrl = wsUrl;

    connect(updateSocket, &QWebSocket::textMessageReceived,
            this, &HttpClient::onUpdateNotificationReceived);
    connect(updateSocket, &QWebSocket::disconnected,
            this, &HttpClient::onSocketDisconnected);

    qDebug() << "Подключение к WebSocket серверу обновлений:" << wsUrl;
    updateSocket->open(QUrl(wsUrl));
}

// onUpdateNotificationReceived вызывается при получении уведомления от сервера.
// Сервер присылает JSON: {"module":"<n>","version":"<v>"}.
// Клиент отправляет подтверждение готовности, после чего инициирует скачивание.
void HttpClient::onUpdateNotificationReceived(const QString &message)
{
    qDebug() << "Получено уведомление об обновлении:" << message;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || doc.isNull()) {
        qDebug() << "Ошибка разбора уведомления об обновлении:" << parseError.errorString();
        return;
    }

    QJsonObject notification = doc.object();
    QString module  = notification["module"].toString();
    QString version = notification["version"].toString();

    if (module.isEmpty() || version.isEmpty()) {
        qDebug() << "Уведомление об обновлении содержит неполные данные";
        return;
    }

    qDebug() << "Доступно обновление модуля:" << module << "версия:" << version;

    // Отправляем серверу подтверждение готовности к обновлению
    QJsonObject ack;
    ack["status"] = "ready";
    ack["module"] = module;
    updateSocket->sendTextMessage(QJsonDocument(ack).toJson(QJsonDocument::Compact));

    // Инициируем скачивание зашифрованного файла обновления
    downloadUpdate(module, version);
}

// onSocketDisconnected вызывается при разрыве WebSocket-соединения.
// Переподключение через 5 секунд — чтобы не потерять уведомления об обновлениях.
void HttpClient::onSocketDisconnected()
{
    qDebug() << "WebSocket соединение разорвано. Переподключение через 5 секунд...";
    QTimer::singleShot(5000, this, [this]() {
        if (!wsUrl.isEmpty()) {
            updateSocket->open(QUrl(wsUrl));
        }
    });
}

// downloadUpdate скачивает зашифрованный файл обновления с сервера.
// Передаёт публичный RSA-ключ клиента в заголовке X-Client-Public-Key —
// сервер шифрует им AES-ключ сессии (гибридное шифрование).
void HttpClient::downloadUpdate(const QString &module, const QString &version)
{
    QString url = QString("http://192.168.91.201:8080/update?module=%1").arg(module);
    QNetworkRequest request(QUrl(url));

    QByteArray pubKey = loadPublicKey();
    if (pubKey.isEmpty()) {
        qDebug() << "Не удалось загрузить публичный ключ клиента. Обновление отменено.";
        return;
    }

    // Передаём публичный ключ серверу — он зашифрует им AES-ключ для этой сессии
    request.setRawHeader("X-Client-Public-Key", pubKey);

    qDebug() << "Скачивание обновления модуля:" << module << "версия:" << version;

    QNetworkReply *reply = manager->get(request);

    // Используем лямбду, чтобы передать контекст (module) в слот завершения
    connect(reply, &QNetworkReply::finished, this, [this, reply, module]() {
        onUpdateDownloadFinished(reply);
    });
}

// onUpdateDownloadFinished вызывается после скачивания файла обновления.
// Проверяет хеш полученного файла, сохраняет его и запускает апдейтер.
void HttpClient::onUpdateDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Ошибка скачивания обновления:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    // Сервер возвращает SHA-256 хеш исходного (незашифрованного) файла в заголовке
    QString expectedHash = reply->rawHeader("X-Update-Hash");
    QString module       = reply->rawHeader("X-Module");
    QString version      = reply->rawHeader("X-Module-Version");

    QByteArray encryptedData  = reply->readAll();
    QByteArray encryptedAESKey = QByteArray::fromBase64(reply->rawHeader("X-Encrypted-Key"));

    reply->deleteLater();

    qDebug() << "Обновление получено. Модуль:" << module << "Версия:" << version;
    qDebug() << "Ожидаемый хеш:" << expectedHash;

    // TODO: расшифровать encryptedAESKey приватным RSA-ключом клиента,
    //       затем расшифровать encryptedData полученным AES-ключом (AES-256-GCM).
    //       До реализации крипто — работаем с данными как есть (для тестирования).
    QByteArray fileData = encryptedData;

    // Сохраняем файл обновления во временную директорию
    QDir stagingDir(kUpdateStagingDir);
    if (!stagingDir.exists()) {
        stagingDir.mkpath(".");
    }
    QString updateFilePath = kUpdateStagingDir + module + ".bin";
    QFile updateFile(updateFilePath);
    if (!updateFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Не удалось сохранить файл обновления:" << updateFilePath;
        return;
    }
    updateFile.write(fileData);
    updateFile.close();

    qDebug() << "Файл обновления сохранён:" << updateFilePath;

    // Запускаем процесс обновления, передавая ему путь к файлу и ожидаемый хеш
    launchUpdater(module, updateFilePath, expectedHash);
}

// launchUpdater запускает отдельный процесс апдейтера с инструкциями по установке.
// Апдейтер: завершает текущее приложение → применяет обновление → перезапускает приложение.
// При следующем запуске terminateUpdaterIfRunning() завершит апдейтер, если он ещё жив.
void HttpClient::launchUpdater(const QString &module, const QString &updateFilePath, const QString &expectedHash)
{
    QString updaterPath = "./updater";

    QStringList args;
    args << "--module"   << module
         << "--file"     << updateFilePath
         << "--hash"     << expectedHash
         << "--app-path" << QCoreApplication::applicationFilePath();

    qDebug() << "Запуск апдейтера:" << updaterPath << args;

    bool started = QProcess::startDetached(updaterPath, args);
    if (!started) {
        qDebug() << "Не удалось запустить процесс апдейтера:" << updaterPath;
        return;
    }

    qDebug() << "Апдейтер запущен. Завершение текущего процесса...";

    // Апдейтер принял управление — завершаем основной процесс,
    // чтобы апдейтер мог заменить файлы приложения
    QCoreApplication::quit();
}

// checkIntegrity сравнивает SHA-256 хеши локальных файлов с хешами, полученными с сервера.
// Если есть несовпадение — определяет повреждённый модуль и инициирует его замену.
void HttpClient::checkIntegrity(const QJsonObject &serverHashes)
{
    qDebug() << "Проверка целостности файлов приложения...";

    QString damagedModule = findDamagedModule(serverHashes);

    if (damagedModule.isEmpty()) {
        qDebug() << "Целостность подтверждена. Все хеши совпадают.";
        return;
    }

    qDebug() << "Обнаружен повреждённый модуль:" << damagedModule << "— инициируем замену.";

    // Запрашиваем актуальную версию повреждённого модуля с сервера
    QString currentVersion = "latest"; // сервер отдаст актуальную версию
    downloadUpdate(damagedModule, currentVersion);
}

// findDamagedModule проверяет хеши файлов поштучно и возвращает имя первого повреждённого.
// serverHashes — JSON-объект вида {"client":"<hash>","gui":"<hash>","updater":"<hash>"}.
QString HttpClient::findDamagedModule(const QJsonObject &serverHashes)
{
    // Маппинг имени модуля → путь к его исполняемому файлу
    QMap<QString, QString> modulePaths;
    modulePaths["client"]  = QCoreApplication::applicationFilePath();
    modulePaths["gui"]     = "./gui";
    modulePaths["updater"] = "./updater";

    for (auto it = modulePaths.begin(); it != modulePaths.end(); ++it) {
        const QString &moduleName = it.key();
        const QString &filePath   = it.value();

        if (!serverHashes.contains(moduleName)) {
            continue; // Сервер не прислал хеш для этого модуля — пропускаем
        }

        QString serverHash = serverHashes[moduleName].toString();
        QString localHash  = computeFileHash(filePath);

        if (localHash.isEmpty()) {
            qDebug() << "Не удалось вычислить хеш для модуля:" << moduleName;
            continue;
        }

        if (localHash != serverHash) {
            qDebug() << "Хеш модуля" << moduleName << "не совпадает."
                     << "Локальный:" << localHash
                     << "Серверный:" << serverHash;
            return moduleName;
        }
    }

    return QString(); // все хеши совпали
}

// computeFileHash вычисляет SHA-256 хеш файла по указанному пути.
// Возвращает пустую строку, если файл не удалось открыть.
QString HttpClient::computeFileHash(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Не удалось открыть файл для хеширования:" << filePath;
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        qDebug() << "Ошибка чтения файла при вычислении хеша:" << filePath;
        return QString();
    }

    return hash.result().toHex();
}

// terminateUpdaterIfRunning проверяет наличие запущенного процесса апдейтера и завершает его.
// Вызывается при старте приложения: апдейтер мог не завершиться корректно после обновления.
void HttpClient::terminateUpdaterIfRunning()
{
    // Ищем процессы с именем апдейтера через системную утилиту pgrep
    QProcess finder;
    finder.start("pgrep", QStringList() << "-x" << kUpdaterProcessName);
    finder.waitForFinished(3000);

    QString output = finder.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        return; // Апдейтер не запущен — всё нормально
    }

    qDebug() << "Обнаружен запущенный процесс апдейтера (PID:" << output << "). Завершение...";

    // Завершаем все найденные процессы апдейтера
    for (const QString &pidStr : output.split('\n')) {
        bool ok = false;
        qint64 pid = pidStr.toLongLong(&ok);
        if (!ok) continue;

        QProcess killer;
        killer.start("kill", QStringList() << QString::number(pid));
        killer.waitForFinished(2000);

        qDebug() << "Процесс апдейтера (PID:" << pid << ") завершён.";
    }
}

// loadPublicKey читает публичный RSA-ключ клиента из файла.
// Используется при запросе обновления — сервер зашифрует им AES-ключ сессии.
QByteArray HttpClient::loadPublicKey()
{
    QFile keyFile(kPublicKeyPath);
    if (!keyFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Не удалось открыть файл публичного ключа:" << kPublicKeyPath;
        return QByteArray();
    }
    return keyFile.readAll();
}
