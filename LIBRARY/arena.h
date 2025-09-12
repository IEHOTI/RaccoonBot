#ifndef ARENA_H
#define ARENA_H

#include "Task.h"

class BOTLIB_EXPORT Arena : public Task
{
public:
    explicit Arena(Controller *g_controller, QObject *parent = nullptr);
    ~Arena();

    void Start(ErrorList *result = nullptr) override;
    void Pause() override;
    void Initialize(TaskSettings *setting, ErrorList *result = nullptr) override;
    void Stop() override;
    void checkPower(const Mat &object, ErrorList *result = nullptr) override;
    void setUnitsSet(ErrorList *result = nullptr) override;//
    void checkBattleResult(bool *battle = nullptr) override;

    void confirmSquad(ErrorList *result = nullptr);
    void checkStage(ErrorList *result = nullptr);
    void checkSettings(ErrorList *result = nullptr);
    void savePower(ErrorList *result = nullptr);
    void scanPlayers(ErrorList *result = nullptr);
    void attackPosition(int pos,ErrorList *result = nullptr);
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
