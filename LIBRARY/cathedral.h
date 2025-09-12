#ifndef CATHEDRAL_H
#define CATHEDRAL_H

#include "Task.h"

class BOTLIB_EXPORT Cathedral : public Task
{
public:
    explicit Cathedral(Controller *g_controller, QObject *parent = nullptr);
    ~Cathedral();
    void Start(ErrorList *result = nullptr) override;
    void Pause() override;
    void Initialize(TaskSettings *setting, ErrorList *result = nullptr) override;
    void Stop() override;
    void checkPower(const Mat &object, ErrorList *result = nullptr) override;
    void setUnitsSet(ErrorList *result = nullptr) override;//
    void checkBattleResult(bool *battle = nullptr) override;

    void confirmSquad(ErrorList *result = nullptr);
    void checkMain(ErrorList *result = nullptr);
    void checkStage(ErrorList *result = nullptr);
    void checkSettings(ErrorList *result = nullptr);
    void checkWarnings();
    void findWaypoint(ErrorList *result = nullptr);
    void checkEndStage(ErrorList *result = nullptr);
    void checkWaypoints(int &type, ErrorList *result = nullptr);
    void attackWaypoints(int type, ErrorList *result = nullptr, bool *battle = nullptr);
    void fullGamePass(ErrorList *result = nullptr);
    void safePower(ErrorList *result = nullptr);//это при выставлении юнитов в mode собственный отряд
    void bossGamePass(ErrorList *result = nullptr);//
    QVector<Rect> getBossWay(Rect &rect);
private:
    QStringList nameWaypoints;
    CathedralSettings *settings;
    int currentStage;
};

#endif // CATHEDRAL_H
