#ifndef ARENA_H
#define ARENA_H

#include "Task/Task.h"
#include "Arena/ArenaPlayer.h"

struct ArenaSettings;

class BOTLIB_EXPORT Arena : public Task
{
public:
    explicit Arena(Controller *g_controller, QObject *parent = nullptr);
    ~Arena();

    //Task act
    void Start(ErrorList *result = nullptr) override;
    void Pause() override;
    void Initialize(TaskSettings *setting, ErrorList *result = nullptr) override;
    void Stop() override;

    //PreArena act
    void setUnitsSet(ErrorList *result = nullptr) override;//
    void checkPower(const cv::Mat &object, ErrorList *result = nullptr) override;
    void confirmSquad(ErrorList *result = nullptr);
    void checkSettings(ErrorList *result = nullptr);
    void savePower(ErrorList *result = nullptr);

    //Arena act
    void checkBattleResult(bool *battle = nullptr) override;
    void checkStage(ErrorList *result = nullptr);
    void scanPlayers(ErrorList *result = nullptr);
    void attackPosition(int pos,ErrorList *result = nullptr);
    void waitFind();
    void printPlayers();

    //Stubs
    void savePlayerToBlackList(const cv::Mat &player, int playerPower);
    void savePlayerToWhileList(const cv::Mat &player, int playerPower);
    void loadLists(int playerID); //playerID?
private:
    int currentStage;
    int maxPower;
    ArenaPlayer me;
    ArenaSettings *settings;
    QList<ArenaPlayer> listPlayers;
    QList<cv::Mat> blackList;
    QList<cv::Mat> whiteList;
};

#endif // ARENA_H
