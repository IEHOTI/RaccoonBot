#ifndef ARENASTRATEGY_H
#define ARENASTRATEGY_H

#include "BotLib_global.h"

struct ArenaPlayer;

struct BOTLIB_EXPORT ArenaStrategy {
    bool check(const ArenaPlayer &temp, int maxPower);

    //тут приоритеты выписать
    int getPosition(const QList<ArenaPlayer> &listPlayers, int stage, int maxPower);

    //false = ниже себя, true = выше себя
    bool first = false;
    bool second = false;
    bool third = false;
    bool fourth = false;
    bool fives = false;
    //false = ignore, true =not
    bool black = true;
    bool white = false;
};

#endif // ARENASTRATEGY_H
