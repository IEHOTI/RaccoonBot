#ifndef LIGHTHOUSE_H
#define LIGHTHOUSE_H
#include "Task.h"

class BOTLIB_EXPORT Lighthouse : public Task
{
public:
    explicit Lighthouse(Controller *g_controller, QObject *parent = nullptr);
    ~Lighthouse();

    void Start(ErrorList *result = nullptr) override;
    void Pause() override;
    void Initialize(TaskSettings *setting, ErrorList *result = nullptr) override;
    void Stop() override;
    void checkPower(const Mat &object, ErrorList *result = nullptr) override;
    void setUnitsSet(ErrorList *result = nullptr) override;
    void checkBattleResult(bool *battle = nullptr) override;

    void checkWarning();
    void scanEnemy(int &attack_pos, Mat &resultScan, ErrorList *result = nullptr); // тут ищется возможная атака и запоминается позиция
    void attackEnemy(int pos,int &count, ErrorList *result = nullptr);
    void refreshEnemy(int &count_attack,int count_lose, int &count_refresh, ErrorList *result = nullptr);
    void saveHistoryPower(ErrorList *result = nullptr);//

    void savePlayerToBlackList(const Mat &player, int playerPower);
    void savePlayerToWhileList(const Mat &player, int playerPower);
    void loadLists(int playerID); //playerID?

private:
    QList<Mat> blackList;
    QList<Mat> whiteList;
    LighthouseSettings *settings;
    int myPower;
};

#endif // LIGHTHOUSE_H
