#ifndef ARENASETTINGS_H
#define ARENASETTINGS_H

#include "Task/TaskSettings.h"
#include "Arena/ArenaStrategy.h"
#include <QList>

struct BOTLIB_EXPORT ArenaSettings : public TaskSettings {
    QList<typeSet> squadSet;
    int modeSquad = 0; // 0 - better unit, 1 - last squad, 2 - my squad
    int modeTicket = 1;  // 0 - apple, 1 - Key, 2 - apple > Key, 3 - Key > apple
    int saveApple = 0;
    double rangePower = 1.0; // power*1.0
    ArenaStrategy strategy;
};

#endif // ARENASETTINGS_H
