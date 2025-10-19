#ifndef CATHEDRAL_H
#define CATHEDRAL_H

#include "Task/Task.h"

namespace cv {
template<typename _Tp> class Rect_;
typedef Rect_<int> Rect;
}

struct CathedralSettings;

class BOTLIB_EXPORT Cathedral : public Task
{
public:
    explicit Cathedral(Controller *g_controller, QObject *parent = nullptr);
    ~Cathedral();

    //Task
    void Start(ErrorList *result = nullptr) override;
    void Pause() override;
    void Initialize(TaskSettings *setting, ErrorList *result = nullptr) override;
    void Stop() override;

    //PreCathedral
    void checkMain(ErrorList *result = nullptr);
    void confirmSquad(ErrorList *result = nullptr);
    void checkSettings(ErrorList *result = nullptr);
    QVector<cv::Rect> getBossWay(cv::Rect &rect);

    //Cathedral
    void checkWarnings();
    void checkPower(const cv::Mat &object, ErrorList *result = nullptr) override;
    void checkBattleResult(bool *battle = nullptr) override;
    void checkStage(ErrorList *result = nullptr);
    void findWaypoint(ErrorList *result = nullptr);
    void checkEndStage(ErrorList *result = nullptr);
    void checkWaypoints(int &type, ErrorList *result = nullptr);
    void attackWaypoints(int type, ErrorList *result = nullptr, bool *battle = nullptr);
    void fullGamePass(ErrorList *result = nullptr);
    void bossGamePass(ErrorList *result = nullptr);

    //Stubs
    void setUnitsSet(ErrorList *result = nullptr) override;//
    void safePower(ErrorList *result = nullptr);//это при выставлении юнитов в mode собственный отряд

private:
    QStringList nameWaypoints;
    CathedralSettings *settings;
    int currentStage;
};

#endif // CATHEDRAL_H
