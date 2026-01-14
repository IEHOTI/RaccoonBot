#include "MainWindow/MainWindow.h"
#include "MyClasses/GeneralData.h"
#include "MyClasses/DynamicComboBox.h"
#include "MainPage/MainPageSerializer.h"

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>
#include <QFile>

#include "Controller/Controller.h"
#include "User/UserProfile.h"
#include "qdir.h"

void MainWindow::createMainTab(QWidget *tab,int index) {
    if (!listData[index]) {
        QString errorMsg = QString("Неверный индекс массива (%1). Допустимый диапазон: [0; %2]")
                               .arg(index)
                               .arg(listData.count() - 1);
        if (!listErrorLogs[index]) QMessageBox::critical(this, "Ошибка", errorMsg, QMessageBox::Ok);
        else listErrorLogs[index]->setText(errorMsg);
        return;
    }

    listData[index]->accountInfo = new QWidget(tab); // получение виджета инфы об аккаунте
    listData[index]->user->getInfo(listData[index]->accountInfo); // userProfile user
    listData[index]->accountInfo->setGeometry(0,0, 380, 200);

    DynamicComboBox* listEmulators = new DynamicComboBox(tab);
    listEmulators->setGeometry(5, 210, 250, 25);

    //через сигналы обновлять буду
    QLabel* statusLabel = new QLabel(tab);
    statusLabel->setText("Статус бота: Отключен");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setGeometry(5,245,250,25);

    connect(listData[index]->controller,&Controller::Logging,this,[=](const QString &msg, const bool print){
        if(print) statusLabel->setText(msg);
    },Qt::QueuedConnection);

    QPushButton* startButton = new QPushButton("Старт",tab);
    startButton->setGeometry(5,280,120,30);
    startButton->setStyleSheet("QPushButton {"
                               "background-color:gray;"
                               "color: white;"
                               "margin: 0px;"
                               "padding: 0px;"
                               "border: none;"
                               "text-align: center;"
                               "}");
    startButton->setEnabled(false);

    QPushButton* stopButton = new QPushButton("Стоп",tab);
    stopButton->setGeometry(135,280,120,30);
    stopButton->setStyleSheet("QPushButton {"
                              "background-color:gray;"
                              "color: white;"
                              "margin: 0px;"
                              "padding: 0px;"
                              "border: none;"
                              "text-align: center;"
                              "}");
    stopButton->setEnabled(false);

    QLabel* labelRaccoon = new QLabel(tab);
    labelRaccoon->setGeometry(255,195,130,130);
    labelRaccoon->setPixmap(QPixmap(":/pages/what.png"));

    connect(listEmulators, &DynamicComboBox::currentIndexChanged, this, [=](int localIndex){
        if (localIndex > 0 && localIndex < listEmulators->count()) {
            listData[index]->user->emulator_name = listEmulators->itemText(localIndex);
            startButton->setEnabled(true);
            startButton->setStyleSheet("background-color:green; color:white;");
        }
        else {
            startButton->setEnabled(false);
            startButton->setStyleSheet("background-color:gray; color:white;");
            stopButton->setEnabled(false);
            stopButton->setStyleSheet("background-color:gray; color:white;");
        }
    });

    connect(startButton,&QPushButton::clicked,this,[=](){
        if(usedEmulators.contains(listEmulators->currentText())) {
            listData[index]->controller->Logging("Выбранный эмулятор " + listEmulators->currentText() + " уже занят другим ботом.",false);
            listEmulators->setCurrentIndex(0);
            listData[index]->controller->Logging("Выберите другой эмулятор.");
            return;
        }
        usedEmulators.append(listEmulators->currentText());
        listErrorLogs[index]->clear();
        if(listData[index]->user->bot_ID == 0) listData[index]->user->bot_ID = (rand() % 10000);
        labelRaccoon->setPixmap(QPixmap(":/pages/anger.png"));
        startButton->setEnabled(false);
        listEmulators->setEnabled(false);
        startButton->setStyleSheet("background-color:gray; color:white;");
        stopButton->setEnabled(true);
        stopButton->setStyleSheet("background-color:red; color:white;");
        botThreads[index]->start();
        QMetaObject::invokeMethod(listData[index]->controller,"Start",Qt::QueuedConnection,
                                  Q_ARG(userProfile*,listData[index]->user),Q_ARG(ErrorList*,nullptr));
    });

    connect(listData[index]->controller,&Controller::endStart,this,[=]() {
        // Удаляем старый виджет
        if (listData[index]->accountInfo) {
            listData[index]->accountInfo->hide();
            listData[index]->accountInfo->deleteLater();
            listData[index]->accountInfo = nullptr;
        }
        // Создаем новый
        listData[index]->accountInfo = new QWidget(tab);
        listData[index]->accountInfo->setGeometry(4,0,380,200);
        listData[index]->user->getInfo(listData[index]->accountInfo);
        listData[index]->accountInfo->show();

        if(listData[index]->user->subscribe == typeSub::admin) {
            listErrorLogs[index]->show();
            labelRaccoon->setPixmap(QPixmap(":/pages/cute.png"));
        }

        tabWidget->setTabText(index,QString::number(listData[index]->user->user_ID));

        emit listData[index]->controller->saveTaskQueue(index);
    },Qt::QueuedConnection);

    connect(listData[index]->controller,&Controller::stopStart,listData[index]->controller,&Controller::Stop,Qt::QueuedConnection);

    connect(stopButton,&QPushButton::clicked,this,[=]{
        usedEmulators.removeAll(listEmulators->currentText());
        labelRaccoon->setPixmap(QPixmap(":/pages/what.png"));
        stopButton->setEnabled(false);
        stopButton->setStyleSheet("background-color: gray; color: white");
        startButton->setEnabled(true);
        listEmulators->setEnabled(true);
        startButton->setStyleSheet("background-color: green");
        if(listData[index]->currentTask < 0) emit listData[index]->controller->stopStart();
        else emit listData[index]->stopTask();
        botThreads[index]->wait();
    });

    connect(this, &MainWindow::loadMainPage,this,[=](){
        //предполагается, что память о прошлом "юзере" почистилась в "верхнем уровне функции", почистился только userProfile & Emulator
        if (listData[index]->accountInfo) {
            listData[index]->accountInfo->hide();
            listData[index]->accountInfo->deleteLater();
            listData[index]->accountInfo = nullptr;
        }
        // Создаем новый
        listData[index]->accountInfo = new QWidget(tab);
        listData[index]->accountInfo->setGeometry(4, 0, 380, 200);
        listData[index]->user->getInfo(listData[index]->accountInfo, "color: orange;");
        listData[index]->accountInfo->show();
        emit stopButton->clicked();
        listEmulators->setMyText(listData[index]->user->emulator_name);
        tabWidget->setTabText(index,QString::number(listData[index]->user->user_ID));
    },Qt::QueuedConnection);
}

void MainWindow::serializeMainPage(int index) {
    QString path{"user/" + QString::number(listData[index]->user->user_ID)};
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать папку " + QDir::current().filePath(path), QMessageBox::Ok);
            return; // или обработка ошибки
        }
    }
    QByteArray byteArray = MainPageSerializes::toBinary(listData[index]->user,listData[index]->emulator);
    QFile file(path + "/acc.mrc");
    // Открываем файл перед записью
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
        return;
    }
    file.write(byteArray);
    file.close();
}

void MainWindow::unSerializeMainPage(const QString &accountId, int index) {
    QFile file("user/" + accountId + "/acc.mrc");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
        return;
    }

    QByteArray byteArray = file.readAll();
    file.close();

    bool result = MainPageSerializes::fromBinary(byteArray, listData[index]->user, listData[index]->emulator);
    if(!result) {
        qWarning() << "Ошбика десериализации";
        return;
    }
    emit loadMainPage();
}

