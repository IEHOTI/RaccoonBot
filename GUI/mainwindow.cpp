#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->hide();
    setWindowTitle("RaccoonBot");
    resize(390,400);
    setFixedSize(390,400);
    setCentralWindow(390,400);

    preProcessingData();
    preProcessingMenuBar();
}

MainWindow::~MainWindow() {
    //аккуратно следить за памятью
    QThread* mainThread = QCoreApplication::instance()->thread();

    for (int i = 0; i < listData.size(); ++i) {
        auto& data = listData[i];
        if (!data) continue;

        QThread* currentThread = data->thread();
        // Переносим объект обратно в главный поток, если он не там
        if (currentThread != mainThread) {
            QMetaObject::invokeMethod(data.data(), [data]() {
                data->moveToThread(QCoreApplication::instance()->thread());
            }, Qt::BlockingQueuedConnection);
        }
    }

    // Завершаем потоки
    for (QThread* thread : botThreads) {
        if (!thread)
            continue;

        thread->quit();
        thread->wait();  // блокируемся, пока поток завершится
        delete thread;
    }

    botThreads.clear();
    listData.clear();  // теперь можно безопасно удалить GeneralData

}

void MainWindow::setCentralWindow(int width, int height){
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int x = (screenGeometry.width() - width) / 2;
    int y = (screenGeometry.height() - height) / 2;

    this->move(x, y);
}

void MainWindow::preProcessingData() {
    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    nameTasks << "Арена" << "Башня" << "Бухта" << "Грабежи" << "Ивент"
              << "Колизей" << "Маяк" << "Подземелье" << "Портал" << "Разное"
              << "Собор";
    for(int i = 0, n = nameTasks.size(); i < n; i++) hashTask[nameTasks[i]] = i;
    //в будущем загрузка сохраненных вкладок и прочего, пока что всегда новое
    tabWidget = new QTabWidget(mainWidget);
    QVBoxLayout* layout = new QVBoxLayout(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(tabWidget);

    tempData = QSharedPointer<GeneralData>::create();

    Controller *controller = new Controller(tempData.data());
    QTextEdit *errorLog = new QTextEdit();
    Ocr *ocr = new Ocr((path)"G:\\Coding\\Photo\\test\\numbers",tempData.data());
    ocr->Initialize();
    userProfile *user = new userProfile();
    QThread *thread = new QThread(this);
    thread->start();
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
    connect(controller,&Controller::errorLogging,errorLog,&QTextEdit::append,Qt::QueuedConnection);
    connect(controller,&Controller::Recognize,ocr,&Ocr::Recognize);
    connect(controller,&Controller::emulatorCreated,tempData.data(),&GeneralData::saveEmulator,Qt::QueuedConnection);
    connect(this,&MainWindow::startController,controller,&Controller::Start,Qt::QueuedConnection);
    connect(this,&MainWindow::startBot,tempData.data(),&GeneralData::executeTasks,Qt::QueuedConnection);
    connect(ocr,&Ocr::sendError,errorLog,&QTextEdit::append,Qt::QueuedConnection);
    createGUI();
    this->show();
}

void MainWindow::preProcessingMenuBar() {

    // Меню-бары
    QMenuBar* menuBar = new QMenuBar(mainWidget);

    //Аккаунты
    QMenu* generalMenu = new QMenu("Главная", mainWidget);
    QAction* addAccountAction = new QAction("Добавить аккаунт", mainWidget);
    QAction* removeAccountAction = new QAction("Удалить аккаунт", mainWidget);
    QAction* statisticAccountAction = new QAction("Статистика",mainWidget);
    QAction* saveAccountSetting = new QAction("Сохранить настройки",mainWidget);
    QAction* loadAccountSetting = new QAction("Загрузить настройки",mainWidget);

    generalMenu->addAction(addAccountAction);
    generalMenu->addAction(removeAccountAction);
    generalMenu->addAction(statisticAccountAction);
    generalMenu->addAction(saveAccountSetting);
    generalMenu->addAction(loadAccountSetting);

    //О программе
    QMenu* aboutProgram = new QMenu("О программе",mainWidget);

    //Инструменты бота
    QMenu* botMenu = new QMenu("Инструменты бота",this);
    QAction* clearLog = new QAction("Очистка лога",this);
    QAction* eyeBot = new QAction("Зрение бота",this);
    QAction* screenBot = new QAction("Сделать скриншот эмулятора",this);
    QAction* sizeFix = new QAction("Исправить размер эмулятора",this);
    QAction* winFix = new QAction("Исправить масштаб экрана",this);
    QAction* posFix = new QAction("Исправить местоположение эмулятора",this);
    QAction* fullFix = new QAction("Полное исправление бота с перезагрузкой игры",this);
    QAction* settingBot = new QAction("Настройки бота",this);

    botMenu->addAction(clearLog);
    botMenu->addAction(eyeBot);
    botMenu->addAction(screenBot);
    botMenu->addAction(sizeFix);
    botMenu->addAction(winFix);
    botMenu->addAction(posFix);
    botMenu->addAction(fullFix);
    botMenu->addAction(settingBot);//Там будет автозапуск, вырубание бота при бездействии и всякая хрень

    //Связь со мной
    QMenu* linkMenu = new QMenu("Связь со мной", this);
    QAction* TgChannel = new QAction("Телеграмм-канал бота",this);
    QAction* DsChannel = new QAction("Дискорд чат(недоступен в РФ)",this);

    linkMenu->addAction(TgChannel);
    linkMenu->addAction(DsChannel);

    // Лицензия программы
    QAction* myAbout = new QAction("Лицензии программы",mainWidget);

    aboutProgram->addMenu(botMenu);
    aboutProgram->addMenu(linkMenu);
    aboutProgram->addAction(myAbout);

    //Прочие действия без меню
    QAction* manual = new QAction("Инструкция",this);
    QAction* bugReport = new QAction("Сообщить об ошибке",this);

    menuBar->addMenu(generalMenu);
    menuBar->addAction(manual);
    menuBar->addMenu(aboutProgram);
    menuBar->addAction(bugReport);

    setMenuBar(menuBar);
}

void MainWindow::createGUI() {
    QWidget* widget = new QWidget(tabWidget);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    QTabWidget* taskTabWidget = new QTabWidget(widget);
    int index = tabWidget->count();

    // Добавление 4 poka chto под-вкладок
    for (int i = 0; i < 5; ++i) {
        QWidget* tab = new QWidget(taskTabWidget);
        switch(i) {
        case 0:{
            createMainTab(tab,index);
            taskTabWidget->addTab(tab, "Главная");
            break;
        }
        case 1:{
            createGeneralTab(tab,index);
            taskTabWidget->addTab(tab, "Общее");
            break;
        }
        case 2:{
            createArenaTab(tab,index);
            taskTabWidget->addTab(tab, "Арена");
            break;
        }
        case 3:{
            createCathedralTab(tab,index);
            taskTabWidget->addTab(tab, "Собор");
            break;
        }
        case 4:{
            createLoggerTab(tab,index);
            taskTabWidget->addTab(tab, "Лог-вкладка");
            break;
        }

        }
    }
    tempData.clear();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(taskTabWidget);
    widget->setLayout(layout);
    tabWidget->addTab(widget, "Новый аккаунт");
}

void MainWindow::createLoggerTab(QWidget *tab,int index) {
    userProfile *tempUser = listData[index]->user;
    tempUser->logger = new QTextEdit(tab);
    tempUser->logger->setGeometry(5,5,372,307);
    connect(listData[index]->controller,&Controller::Logging,this,[=](const QString &msg){
        tempUser->logger->append("[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] " + msg);
    },Qt::QueuedConnection);
    //если надо потом кнопки очистки добавлю и прочее.
}
