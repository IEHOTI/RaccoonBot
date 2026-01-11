#ifndef TASKSETTINGS_H
#define TASKSETTINGS_H

#include "BotLib_global.h"

enum class typeSet {
    NOT_TOUCH,
    SET_1,
    SET_2,
    SET_3,
    UNEQUIP,
    CUSTOM,
};

struct BOTLIB_EXPORT TaskSettings {
    int history_power = 0;
    int count = 1;
    bool watchADS = false;
    bool premiumStatus = false;
    bool openChest = false;

    virtual ~TaskSettings() = default;
};

#endif // TASKSETTINGS_H
