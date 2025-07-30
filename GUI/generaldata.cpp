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
    int i = 0;
    connect(controller,&Controller::SuccessFix,this,[this,&i](){
        i--;
    });
    //connect(this, &MainWindow::startCathedral, cathedral, &Cathedral::Start, Qt::QueuedConnection); - НЕ НУЖЕН?
    bool result = false;
    while(true) {
        result = false;
        for(int n = listTaskQueue.size(); i < n; i++) {
            int curIndex = hashTasks[listTaskQueue[i]];
            listTasks[curIndex]->Start(&result);
            if(!result) {
                controller->Logging("Задание " + listTaskQueue[i] + " провалено.");
                break;
            }
        }
        if(!result) break;
        i = 0;
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
