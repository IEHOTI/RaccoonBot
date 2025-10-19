#ifndef LIGHTHOUSESETTINGS_H
#define LIGHTHOUSESETTINGS_H

#include "Task/TaskSettings.h"
#include <QList>

struct BOTLIB_EXPORT LighthouseSettings : public TaskSettings {
    int modeRefresh = -1; // -1 = first lose, 3 = after 3 attack, 5 = after 5 attack
    QList<typeSet> squadSet;
    int diamondRefresh = -1;
    int diamondAttack = -1;
    int user_power = 0;
    double rangePower = 1.0;
    //false = ignore, true =not
    bool black = true;
    bool white = false;
};
#endif // LIGHTHOUSESETTINGS_H
