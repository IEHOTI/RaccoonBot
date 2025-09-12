#ifndef TASK_H
#define TASK_H
#include "Controller.h"
#include "TaskSettings.h"
#include "Error.h"

class BOTLIB_EXPORT Task : public QObject {
    Q_OBJECT
public:
    explicit Task(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Task() = default;
    virtual void Start(ErrorList *result = nullptr) = 0;
    virtual void Pause() = 0;
    virtual void Initialize(TaskSettings *setting, ErrorList *result = nullptr) = 0;
    virtual void Stop() = 0;
    virtual void checkPower(const Mat &object, ErrorList *result = nullptr) = 0;
    virtual void setUnitsSet(ErrorList *result = nullptr) = 0;
    virtual void checkBattleResult(bool *battle = nullptr) = 0;

protected:
    Controller *controller;
    QString localPath;
    bool stop_flag;
};

#endif // TASK_H
