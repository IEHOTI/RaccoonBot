#ifndef ARENAPLAYER_H
#define ARENAPLAYER_H

#include "BotLib_global.h"

enum class State : short {
    UNKNOWN,
    IS_ME,
    AVAILABLE,
    VICTORY,
    DEFEAT,
};

struct BOTLIB_EXPORT ArenaPlayer {
    bool blackList = false;
    bool whiteList = false;
    int place = -1;
    int power = 0;
    State status = State::UNKNOWN;

    ArenaPlayer &operator=(const ArenaPlayer &temp){
        this->blackList = temp.blackList;
        this->whiteList = temp.whiteList;
        this->place = temp.place;
        this->power = temp.power;
        this->status = temp.status;
        return *this;
    }
};


#endif // ARENAPLAYER_H
