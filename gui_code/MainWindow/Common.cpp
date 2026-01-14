#include "MainWindow.h"
#include "MyClasses/GeneralData.h"
#include "Controller/Controller.h"

#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#include <QThread>
#include <QDir>
#include <QFile>
#include <QMessageBox>

void MainWindow::setCentralWindow(int width, int height){
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int x = (screenGeometry.width() - width) / 2;
    int y = (screenGeometry.height() - height) / 2;

    this->move(x, y);
}

void MainWindow::removeAccount() {
    int index = tabWidget->currentIndex();
    GeneralData *tempLocalData = listData[index].data();
    QThread *tempThread = QThread::currentThread(); // Основной поток
    tabWidget->removeTab(index);
    QMetaObject::invokeMethod(tempLocalData, [tempLocalData, tempThread]() {
        tempLocalData->moveToThread(tempThread);
    }, Qt::BlockingQueuedConnection); // Блокируем, чтобы дождаться завершения

    emit tempLocalData->stopTask();

    botThreads[index]->quit();
    botThreads[index]->wait();

    listErrorLogs[index]->clear();
    listData[index].clear();
    delete botThreads[index];
    botThreads[index] = nullptr;
    listErrorLogs[index]->deleteLater();
    listErrorLogs[index] = nullptr;

    botThreads.removeAt(index);
    listErrorLogs.removeAt(index);
    listData.removeAt(index);

    if(listData.size() == 0) preProcessingData();
}

void MainWindow::saveData() {
    int userId = 69334893; //later user
    emit serializeApplication(userId);
    int index = tabWidget->currentIndex();
    serializeMainPage(index);
}
void MainWindow::loadData() {
    int userId = 69334893; //later user
    emit unSerializeApplication(userId);
    int index = tabWidget->currentIndex();
    unSerializeMainPage("69334893", index); // temp ""
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

