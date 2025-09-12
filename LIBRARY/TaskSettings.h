#ifndef TASKSETTINGS_H
#define TASKSETTINGS_H
#include "Structs.h"

struct BOTLIB_EXPORT TaskSettings {
    int history_power = 0;
    int count = 1;
    bool watchADS = false;
    bool premiumStatus = false;
    bool openChest = false;

    virtual ~TaskSettings() = default;
};

struct BOTLIB_EXPORT CathedralSettings : public TaskSettings {
    int modeDifficult = 0; // 0 - easy, 1 - hard, 2 - insane
    int modeKey = 1; // 0 - apple, 1 - Key, 2 - apple > Key, 3 - Key > apple
    int modeSquad = 0; // 0 - better unit, 1 - last squad, 2 - my squad
    bool fullGamePass = true;
    QList<typeSet> squadSet;
    int saveApple = 0;
};

struct BOTLIB_EXPORT ArenaSettings : public TaskSettings {
    QList<typeSet> squadSet;
    int modeSquad = 0; // 0 - better unit, 1 - last squad, 2 - my squad
    int modeTicket = 1;  // 0 - apple, 1 - Key, 2 - apple > Key, 3 - Key > apple
    int saveApple = 0;
    double rangePower = 1.0; // power*1.0
    ArenaStrategy strategy;
};

struct BOTLIB_EXPORT LighthouseSettings : public TaskSettings {
    int modeRefresh = -1; // -1 = first lose, 3 = after 3 attack, 5 = after 5 attack
    QList<typeSet> squadSet;
    int diamondRefresh = 0;
    int diamondAttack = 0;
    int user_power = 0;
    double rangePower = 1.0;
    //false = ignore, true =not
    bool black = true;
    bool white = false;
};

#endif // TASKSETTINGS_H
