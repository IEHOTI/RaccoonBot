#include "mainwindow.h"

void MainWindow::createMainTab(QWidget *tab,int index) {
    if (!listData[index]) {
        QString errorMsg = QString("Неверный индекс массива (%1). Допустимый диапазон: [0; %2]")
                               .arg(index)
                               .arg(listData.count() - 1);
        if (!listErrorLogs[index]) QMessageBox::critical(this, "Ошибка", errorMsg, QMessageBox::Ok);
        else listErrorLogs[index]->setText(errorMsg);
        return;
    }
    userProfile *tempUser = listData[index]->user;
    Controller *tempController = listData[index]->controller;

    listData[index]->accountInfo = new QWidget(tab); // получение виджета инфы об аккаунте
    tempUser->getInfo(listData[index]->accountInfo); // userProfile user
    listData[index]->accountInfo->setGeometry(0,0,380,200);

    DynamicComboBox* listEmulators = new DynamicComboBox(tab);
    listEmulators->setGeometry(5, 210, 250, 25);

    //через сигналы обновлять буду
    QLabel* statusLabel = new QLabel(tab);
    statusLabel->setText("Статус бота: Отключен");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setGeometry(5,245,250,25);

    connect(tempController,&Controller::Logging,this,[=](const QString &msg, const bool print){
        if(print) statusLabel->setText(msg);
    },Qt::QueuedConnection);

    QPushButton* startButton = new QPushButton("Старт",tab);
    startButton->setGeometry(5,280,120,30);
    startButton->setStyleSheet("QPushButton {"
                               "background-color:gray;"
                               "color:white;"
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
                              "color:white;"
                              "margin: 0px;"
                              "padding: 0px;"
                              "border: none;"
                              "text-align: center;"
                              "}");
    stopButton->setEnabled(false);

    QLabel* labelRaccoon = new QLabel(tab);
    labelRaccoon->setGeometry(255,195,130,130);
    labelRaccoon->setPixmap(QPixmap(":/pages/what.png"));

    connect(listEmulators, &DynamicComboBox::currentIndexChanged, this, [=](int index){
        if (index > 0 && index < listEmulators->count()) {
            tempUser->emulator_name = listEmulators->itemText(index);
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
        if(tempUser->bot_ID == 0) tempUser->bot_ID = (rand() % 10000);
        labelRaccoon->setPixmap(QPixmap(":/pages/anger.png"));
        startButton->setEnabled(false);
        listEmulators->setEnabled(false);
        startButton->setStyleSheet("background-color:gray; color:white;");
        stopButton->setEnabled(true);
        stopButton->setStyleSheet("background-color:red; color:white;");
        botThreads[index]->start();
        QMetaObject::invokeMethod(listData[index]->controller,"Start",Qt::QueuedConnection,
                                  Q_ARG(userProfile*,tempUser),Q_ARG(ErrorList*,nullptr));
    });

    connect(tempController,&Controller::endStart,this,[=]() {
        // Удаляем старый виджет
        if (listData[index]->accountInfo) {
            listData[index]->accountInfo->hide();
            listData[index]->accountInfo->deleteLater();
            listData[index]->accountInfo = nullptr;
        }
        // Создаем новый
        listData[index]->accountInfo = new QWidget(tab);
        listData[index]->accountInfo->setGeometry(0,0,380,200);
        tempUser->getInfo(listData[index]->accountInfo);
        listData[index]->accountInfo->show();

        if(tempUser->subscribe == typeSub::admin) {
            listErrorLogs[index]->show();
            labelRaccoon->setPixmap(QPixmap(":/pages/cute.png"));
        }

        tabWidget->setTabText(index,QString::number(listData[index]->user->user_ID));

        emit tempController->saveTaskQueue(index);
    },Qt::QueuedConnection);

    connect(tempController,&Controller::stopStart,tempController,&Controller::Stop,Qt::QueuedConnection);

    connect(stopButton,&QPushButton::clicked,this,[=]{
        usedEmulators.removeAll(listEmulators->currentText());
        labelRaccoon->setPixmap(QPixmap(":/pages/what.png"));
        stopButton->setEnabled(false);
        stopButton->setStyleSheet("background-color:gray");
        startButton->setEnabled(true);
        listEmulators->setEnabled(true);
        startButton->setStyleSheet("background-color:green");
        if(listData[index]->currentTask < 0) emit tempController->stopStart();
        else emit listData[index]->stopTask();
        botThreads[index]->wait();
    });

}

void MainWindow::getSettings(int index) {
    //предобработку с учетом подписки юзера сделать
    //сначала эмит в контроллер на сбор эмоций, чисто на секунду задержаться чтобы собрать все дерьмо
    QSharedPointer<GeneralData> localData = listData[index];
    localData->hashTasks.clear();
    localData->listTasks.clear();
    localData->listSettings.clear();
    QList<QString> orderedTask;
    int curId = 0;
    for(const QString &item : localData->listTaskQueue)
        if(!localData->hashTasks.contains(item)) {
            localData->hashTasks[item] = curId++;
            orderedTask.append(item);
        }
    //далее идут эмиты для сбора и сохранения настроек в каждый заданий
    for(const QString& key : orderedTask) {
        int name = hashTask[key];
        // nameTasks << "Арена" 0<< "Башня" 1<< "Бухта" 2<< "Грабежи" 3<< "Ивент" 4
                  // << "Колизей" 5<< "Маяк" 6<< "Подземелье" 7<< "Портал" 8<< "Разное" 9
                  // << "Собор" 10;
        switch (name) {
            //case case case emit emit emit
        case 0:
            emit getArenaSettings(index);
            break;
        case 6:
            emit getLighthouseSettings(index);
            break;
        case 10:
            emit getCathedralSettings(index);
            break;
        default:
            localData->controller->LocalLogging("Ошибка создания задания " + key);
            break;
        }
    }
}
