#ifndef GENERALDATA_H
#define GENERALDATA_H

#include <QObject>
#include <QTextEdit>

#include "TaskSettings.h"
#include "Task.h"
#include "Structs.h"
#include <Ocr.h>

struct GeneralData : public QObject
{
    Q_OBJECT
public:
    ~GeneralData();
    void getCurrentTaskObj(Task *result,int index) {result = listTasks[index];}
    void saveTaskSettings(const QString &strId, bool *result);
    void loadTaskSettings(const QString &strId, bool *result);
    void saveEmulator(Emulator* emulator);
    void executeTasks();
signals:

public:
    QList<Task*> listTasks;
    QList<TaskSettings*> listSettings; //[*] - любая уникалка. порядок заданий определяется и хранится в хеше
    QList<QString> listTaskQueue;
    QHash<QString,int> hashTasks;
    userProfile *user;
    Emulator *emulator;
    Controller *controller;
    Ocr *ocr;
    QWidget *accountInfo;
};

#endif // GENERALDATA_H
