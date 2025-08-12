#include "generaldata.h"

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
    connect(controller,&Controller::SuccessFix,this,[this,&i](){
        currentTask--;
    });
    ErrorList result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(true) {
        currentTask = 0;
        for(int n = listTaskQueue.size(); currentTask < n; currentTask++) {
            QMetaObject::Connection stopConnection;
            stopConnection = connect(this, &GeneralData::stopTask, this, [=]() {
                listTasks[curIndex]->Stop();
            },Qt::QueuedConnection);
            int curIndex = hashTasks[listTaskQueue[currentTask]];
            listTasks[curIndex]->Start(&result);
            if(!result) {
                if(result.error == m_Error::STOP_TASK) {
                    controller->Logging("Задание " + listTaskQueue[currentTask] + " остановлено.");
                    currentTask = -1;
                    return;
                }
                else if(result.error == m_Error::PAUSE_TASK){
                    controller->Logging("Задание " + listTaskQueue[currentTask] + " на паузе.");
                    i--;
                }
                else {
                    controller->Logging("Задание " + listTaskQueue[currentTask] + " провалено.");
                    currentTask = -1;
                    return;
                }
            } else {
                disconnect(stopConnection);
            }
        }
    }
}

void GeneralData::saveEmulator(Emulator *emulator) {
    if(emulator == nullptr) return;
    if(this->emulator != nullptr) delete this->emulator;
    this->emulator = emulator;
}

void GeneralData::saveTaskSettings(const QString &strId, bool *result) {
    //late
}
void GeneralData::loadTaskSettings(const QString &strId, bool *result) {
    //late
}
