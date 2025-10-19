#ifndef LIGHTHOUSE_H
#define LIGHTHOUSE_H

#include "Task/Task.h"

struct LighthouseSettings;

class BOTLIB_EXPORT Lighthouse : public Task
{
public:
    explicit Lighthouse(Controller *g_controller, QObject *parent = nullptr);
    ~Lighthouse();

    //Task
    void Start(ErrorList *result = nullptr) override;
    void Pause() override;
    void Initialize(TaskSettings *setting, ErrorList *result = nullptr) override;
    void Stop() override;

    //PreLighthouse
    void checkPower(const cv::Mat &object, ErrorList *result = nullptr) override;
    void setUnitsSet(ErrorList *result = nullptr) override;

    //Lighthouse
    void checkBattleResult(bool *battle = nullptr) override;
    void checkWarning();
    void scanEnemy(int &attack_pos, cv::Mat &resultScan, ErrorList *result = nullptr); // тут ищется возможная атака и запоминается позиция
    void attackEnemy(int pos,int &count, ErrorList *result = nullptr);
    void refreshEnemy(int &count_attack,int count_lose, int &count_refresh, ErrorList *result = nullptr);

    //Stubs
    void saveHistoryPower(ErrorList *result = nullptr);//
    void savePlayerToBlackList(const cv::Mat &player, int playerPower);
    void savePlayerToWhileList(const cv::Mat &player, int playerPower);
    void loadLists(int playerID); //playerID?

private:
    QList<cv::Mat> blackList;
    QList<cv::Mat> whiteList;
    LighthouseSettings *settings;
    int myPower;
};

#endif // LIGHTHOUSE_H
