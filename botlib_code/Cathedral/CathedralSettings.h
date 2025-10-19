#ifndef CATHEDRALSETTINGS_H
#define CATHEDRALSETTINGS_H

#include "Task/TaskSettings.h"
#include <QList>

struct BOTLIB_EXPORT CathedralSettings : public TaskSettings {
    int modeDifficult = 0; // 0 - easy, 1 - hard, 2 - insane
    int modeKey = 1; // 0 - apple, 1 - Key, 2 - apple > Key, 3 - Key > apple
    int modeSquad = 0; // 0 - better unit, 1 - last squad, 2 - my squad
    bool fullGamePass = true;
    QList<typeSet> squadSet;
    int saveApple = 0;
};

#endif // CATHEDRALSETTINGS_H
