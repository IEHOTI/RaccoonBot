#ifndef GENERALDATA_H
#define GENERALDATA_H

#include <QObject>
#include <QHash>

class Task;
class Emulator;
class Controller;
class Ocr;
struct TaskSettings;
struct userProfile;

struct GeneralData : public QObject
{
    Q_OBJECT
public:
    ~GeneralData();
    void getCurrentTaskObj(Task *result,int index) {result = listTasks[index];}
    //void saveData();
    //void loadData();
    //void saveTaskSettings(const QString &strId, bool *result);
    //void loadTaskSettings(const QString &strId, bool *result);
    void saveEmulator(Emulator* emulator);

public slots:
    void executeTasks();
    void moveOnMainThread(QThread *thread);
signals:
    void stopTask();

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
    int currentTask = -1;
};

#endif // GENERALDATA_H
