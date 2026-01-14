#include "GeneralData.h"

#include <QTextEdit>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QFile>

#include "Task/TaskSettings.h"
#include "Task/Task.h"
#include "Ocr/Ocr.h"
#include "User/UserProfile.h"
#include "Emulators/Core/Emulator.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorList.h"

GeneralData::~GeneralData() {
    qDeleteAll(listTasks);      // Удаляет все Task*
    qDeleteAll(listSettings);   // Удаляет все TaskSettings*
    delete user;
    delete emulator;
    delete controller;
    delete ocr;
    qDebug() << "GeneralData destroyed" << this;
}

void GeneralData::executeTasks(){
    if(listTasks.isEmpty()) {
        controller->Logging("Список заданий пуст.");
        return;
    }
    connect(controller,&Controller::SuccessFix,this,[=](){
        currentTask--;
    });
    ErrorList result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(true) {
        currentTask = 0;
        for(int n = listTaskQueue.size(); currentTask < n; currentTask++) {
            int curIndex = hashTasks[listTaskQueue[currentTask]];
            QMetaObject::Connection stopConnection;
            stopConnection = connect(this, &GeneralData::stopTask, this, [this,&curIndex]() {
                listTasks[curIndex]->Stop();
            },Qt::QueuedConnection);
            listTasks[curIndex]->Start(&result);
            if(!result) {
                if(result.error == m_Error::STOP_TASK) {
                    controller->Logging("Задание " + listTaskQueue[currentTask] + " остановлено.");
                    currentTask = -1;
                    return;
                }
                else if(result.error == m_Error::PAUSE_TASK){
                    controller->Logging("Задание " + listTaskQueue[currentTask] + " на паузе.");
                    currentTask--;
                }
                else {
                    controller->Logging("Задание " + listTaskQueue[currentTask] + " провалено.");
                    currentTask = -1;
                    return;
                }
            } else {
                disconnect(stopConnection);
                //остановить вообще бота
            }
        }
    }
}

void GeneralData::saveEmulator(Emulator *emulator) {
    if(emulator == nullptr) return;
    if(this->emulator != nullptr) delete this->emulator;
    this->emulator = emulator;
}

void GeneralData::moveOnMainThread(QThread *thread) {
    this->moveToThread(thread);
}

// void GeneralData::saveData() {
//     QString userId = QString::number(user->user_ID);
//     QString tempDir("user/" + userId);

//     QJsonObject obj;
//     obj["user_ID"] = user->user_ID;
//     obj["history_power"] = user->history_power;
//     obj["count_units"] = user->count_units;
//     obj["state_premium"] = user->state_premium;
//     obj["state_ads"] = user->state_ads;
//     obj["leftover_time"] = user->leftover_time;
//     obj["emulator_name"] = user->emulator_name;
//     obj["subscribe"] = static_cast<int>(user->subscribe);  // enum в int
//     obj["emulatorType"] = static_cast<int>(user->emulatorType);
//     obj["emulatorPath"] = QString::fromWCharArray(emulator->cmd.c_str());
//     obj["emulatorInstance"] = emulator->instance;

//     QJsonDocument doc(obj);
//     QFile file(tempDir);

//     // Открываем файл перед записью
//     if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//         QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл для записи: " + file.errorString(), QMessageBox::Ok);
//         return;
//     }

//     file.write(doc.toJson(QJsonDocument::Indented));
//     file.close();


// }

// void GeneralData::saveTaskSettings(const QString &strId, bool *result) {
//     //late

// }
// void GeneralData::loadTaskSettings(const QString &strId, bool *result) {
//     //late
//}
