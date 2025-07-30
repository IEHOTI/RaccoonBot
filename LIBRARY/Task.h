#ifndef TASK_H
#define TASK_H
#include "BotLib_global.h"
#include "Controller.h"
#include "TaskSettings.h"
#include "Error.h"

class BOTLIB_EXPORT Task : public QObject {
    Q_OBJECT
public:
    explicit Task(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Task() = default;
    virtual void Start(bool *result = nullptr) = 0;
    virtual void Initialize(TaskSettings *setting, bool *result = nullptr) = 0;
    virtual void Stop() = 0;
    virtual void checkPower(const Mat &object, bool *result = nullptr) = 0;
    virtual void setUnitsSet(bool *result = nullptr) = 0;

protected:
    Controller *controller;
    QString localPath;
    bool stop_flag;
};

#endif // TASK_H
