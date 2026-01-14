#include "MainWindow.h"
#include "MyClasses/GeneralData.h"

#include <QCoreApplication>
#include <QThread>
#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QDateTime>

#include "Controller/Controller.h"
#include "User/UserProfile.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->hide();
    setWindowTitle("RaccoonBot");
    resize(390,400);
    setFixedSize(390,400);
    setCentralWindow(390,400);

    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    tabWidget = new QTabWidget(mainWidget);
    QVBoxLayout *layout = new QVBoxLayout(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(tabWidget);

    nameTasks << "Арена" << "Башня" << "Бухта" << "Грабежи" << "Ивент"
              << "Колизей" << "Маяк" << "Подземелье" << "Портал" << "Разное"
              << "Собор";
    for(int i = 0, n = nameTasks.size(); i < n; i++) hashTask[nameTasks[i]] = i;
    preProcessingData();
    preProcessingMenuBar();
    this->show();
}

MainWindow::~MainWindow() {
    //аккуратно следить за памятью
    QThread *mainThread = QCoreApplication::instance()->thread();

    for (int i = 0; i < listData.size(); ++i) {
        auto& data = listData[i];
        if (!data) continue;

        QThread *currentThread = data->thread();
        // Переносим объект обратно в главный поток, если он не там
        if (currentThread != mainThread) {
            QMetaObject::invokeMethod(data.data(), [data]() {
                data->moveToThread(QCoreApplication::instance()->thread());
            }, Qt::BlockingQueuedConnection);
        }
    }

    // Завершаем потоки
    for (QThread *thread : botThreads) {
        if (!thread)
            continue;

        thread->quit();
        thread->wait();  // блокируемся, пока поток завершится
        delete thread;
    }

    botThreads.clear();
    listData.clear();  // теперь можно безопасно удалить GeneralData

}

void MainWindow::createGUI() {
    QWidget *widget = new QWidget(tabWidget);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QTabWidget *taskTabWidget = new QTabWidget(widget);
    int index = tabWidget->count();

    // Добавление 4 poka chto под-вкладок
    for (int i = 0; i < 6; ++i) {
        QString tabName;
        QWidget *tab = new QWidget(taskTabWidget);
        switch(i) {
        case 0:{
            createMainTab(tab,index);
            taskTabWidget->addTab(tab, "Главная");
            break;
        }
        case 1:{
            tabName = "generalTab";
            createGeneralTab(tab,index);
            taskTabWidget->addTab(tab, "Общее");
            break;
        }
        case 2:{
            tabName = "arenaTab";
            createArenaTab(tab,index);
            taskTabWidget->addTab(tab, "Арена");
            break;
        }
        case 3:{
            tabName = "cathedralTab";
            createCathedralTab(tab,index);
            taskTabWidget->addTab(tab, "Собор");
            break;
        }
        case 4:{
            tabName = "lighthouseTab";
            createLighthouseTab(tab,index);
            taskTabWidget->addTab(tab,"Маяк");
            break;
        }
        case 5:{
            createLoggerTab(tab,index);
            taskTabWidget->addTab(tab, "Лог-вкладка");
            break;
        }

        }
        tab->setObjectName(tabName);
    }
    tempData.clear();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(taskTabWidget);
    widget->setLayout(layout);
    tabWidget->addTab(widget, "Новый аккаунт");

    connect(this, &MainWindow::serializeApplication, this, [=](int userID){
        serializeAllPage(userID,taskTabWidget);
    });
    connect(this, &MainWindow::unSerializeApplication, this, [=](int userID){
        unSerializeAllPage(userID,taskTabWidget);
    });
}

void MainWindow::createLoggerTab(QWidget *tab,int index) {
    userProfile *tempUser = listData[index]->user;
    tempUser->logger = new QTextEdit(tab);
    tempUser->logger->setGeometry(5,5,372,307);

    QFont logFont;
    logFont.setFamily("Tahoma");
    logFont.setPointSize(8);
    tempUser->logger->setFont(logFont);

    connect(listData[index]->controller,&Controller::Logging,this,[=](const QString &msg){
        tempUser->logger->append("[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] " + msg);
        QStringList lines = tempUser->logger->toPlainText().split("\n");
        if (lines.size() > 1000) {
            tempUser->logger->clear();
            tempUser->logger->append("[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] Лог очищен автоматически.");
        }
    },Qt::QueuedConnection);
    //если надо потом кнопки очистки добавлю и прочее.
}
