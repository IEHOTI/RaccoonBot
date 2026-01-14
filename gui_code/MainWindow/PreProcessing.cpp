#include "MainWindow.h"
#include "MyClasses/GeneralData.h"

#include <QThread>
#include <QMenuBar>

#include "Controller/Controller.h"
#include "User/UserProfile.h"
#include "Ocr/Ocr.h"

void MainWindow::preProcessingData() {
    //в будущем загрузка сохраненных вкладок и прочего, пока что всегда новое
    tempData = QSharedPointer<GeneralData>::create();

    Controller *controller = new Controller(tempData.data());
    QTextEdit *errorLog = new QTextEdit();
    Ocr *ocr = new Ocr(tempData.data());
    ocr->Initialize();
    userProfile *user = new userProfile();
    QThread *thread = new QThread(this);
    thread->start();

    QFont logFont;
    logFont.setFamily("Consolas");
    logFont.setPointSize(10);
    logFont.setStyleHint(QFont::Monospace);
    logFont.setFixedPitch(true);
    errorLog->setFont(logFont);
    errorLog->resize(600,600);

    errorLog->show();
    //errorLog->hide();

    tempData->controller = controller;
    tempData->ocr = ocr;
    tempData->user = user;
    tempData->emulator = nullptr;

    tempData->moveToThread(thread);
    listData.append(tempData);
    botThreads.append(thread);
    listErrorLogs.append(errorLog);
    //connect(controller,&Controller::errorLogging,errorLog,&QTextEdit::append,Qt::QueuedConnection);
    connect(controller, &Controller::errorLogging, errorLog, [=](const QString &text) {
        errorLog->append(text);
        QStringList lines = errorLog->toPlainText().split("\n");
        if (lines.size() > 10000) {
            errorLog->clear();
            errorLog->append("=== Лог очищен автоматически ===");
        }
    }, Qt::QueuedConnection);
    connect(controller,&Controller::Recognize,ocr,&Ocr::Recognize);
    connect(controller,&Controller::emulatorCreated,tempData.data(),&GeneralData::saveEmulator,Qt::QueuedConnection);
    connect(ocr,&Ocr::sendError,errorLog,&QTextEdit::append,Qt::QueuedConnection);
    createGUI();
}

void MainWindow::preProcessingMenuBar() {
    // Меню-бары
    QMenuBar *menuBar = new QMenuBar(mainWidget);

    //Аккаунты
    QMenu *generalMenu = new QMenu("Главная", mainWidget);
    QAction *addAccountAction = new QAction("Добавить аккаунт", mainWidget);
    connect(addAccountAction, &QAction::triggered, this, &MainWindow::preProcessingData);
    QAction *removeAccountAction = new QAction("Удалить аккаунт", mainWidget);
    connect(removeAccountAction, &QAction::triggered, this, &MainWindow::removeAccount);
    QAction *statisticAccountAction = new QAction("Статистика",mainWidget);
    QAction *saveAccountSetting = new QAction("Сохранить настройки",mainWidget);
    connect(saveAccountSetting, &QAction::triggered, this, &MainWindow::saveData);
    QAction *loadAccountSetting = new QAction("Загрузить настройки",mainWidget);
    connect(loadAccountSetting, &QAction::triggered, this, &MainWindow::loadData);

    generalMenu->addAction(addAccountAction);
    generalMenu->addAction(removeAccountAction);
    generalMenu->addAction(statisticAccountAction);
    generalMenu->addAction(saveAccountSetting);
    generalMenu->addAction(loadAccountSetting);

    //О программе
    QMenu *aboutProgram = new QMenu("О программе",mainWidget);

    //Инструменты бота
    QMenu *botMenu = new QMenu("Инструменты бота",this);
    QAction *clearLog = new QAction("Очистка лога",this);
    QAction *eyeBot = new QAction("Зрение бота",this);
    QAction *screenBot = new QAction("Сделать скриншот эмулятора",this);
    QAction *sizeFix = new QAction("Исправить размер эмулятора",this);
    QAction *winFix = new QAction("Исправить масштаб экрана",this);
    QAction *posFix = new QAction("Исправить местоположение эмулятора",this);
    QAction *fullFix = new QAction("Полное исправление бота с перезагрузкой игры",this);
    QAction *settingBot = new QAction("Настройки бота",this);

    botMenu->addAction(clearLog);
    botMenu->addAction(eyeBot);
    botMenu->addAction(screenBot);
    botMenu->addAction(sizeFix);
    botMenu->addAction(winFix);
    botMenu->addAction(posFix);
    botMenu->addAction(fullFix);
    botMenu->addAction(settingBot);//Там будет автозапуск, вырубание бота при бездействии и всякая хрень

    //Связь со мной
    QMenu *linkMenu = new QMenu("Связь со мной", this);
    QAction *TgChannel = new QAction("Телеграмм-канал бота",this);
    QAction *DsChannel = new QAction("Дискорд чат(недоступен в РФ)",this);

    linkMenu->addAction(TgChannel);
    linkMenu->addAction(DsChannel);

    // Лицензия программы
    QAction *myAbout = new QAction("Лицензии программы",mainWidget);

    aboutProgram->addMenu(botMenu);
    aboutProgram->addMenu(linkMenu);
    aboutProgram->addAction(myAbout);

    //Прочие действия без меню
    QAction *manual = new QAction("Инструкция",this);
    QAction *bugReport = new QAction("Сообщить об ошибке",this);

    menuBar->addMenu(generalMenu);
    menuBar->addAction(manual);
    menuBar->addMenu(aboutProgram);
    menuBar->addAction(bugReport);

    setMenuBar(menuBar);
}
