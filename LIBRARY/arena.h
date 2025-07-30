#ifndef ARENA_H
#define ARENA_H

#include "Task.h"

class BOTLIB_EXPORT Arena : public Task
{
public:
    explicit Arena(Controller* g_controller, QObject *parent = nullptr);
    ~Arena();

    void Start(bool *result = nullptr) override;
    void Initialize(TaskSettings *setting, bool *result = nullptr) override;
    void Stop() override;
    void checkPower(const Mat &object, bool *result = nullptr) override;
    void setUnitsSet(bool *result = nullptr) override;//

    void confirmSquad(bool *result = nullptr);
    void checkStage(bool *result = nullptr);
    void checkSettings(bool *result = nullptr);
    void savePower(bool *result = nullptr);
    void scanPlayers();
    void attackPosition(int pos,bool *result = nullptr);
    void waitFind();
    void printPlayers();

    void savePlayerToBlackList(const Mat &player, int playerPower);
    void savePlayerToWhileList(const Mat &player, int playerPower);
    void loadLists(int playerID); //playerID?
private:
    int currentStage;
    ArenaPlayer me;
    QList<ArenaPlayer> listPlayers;
    QList<Mat> blackList;
    QList<Mat> whiteList;
    ArenaSettings *settings;
    int maxPower;
};

#endif // ARENA_H
