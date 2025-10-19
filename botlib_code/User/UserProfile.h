#ifndef USERPROFILE_H
#define USERPROFILE_H

#include "BotLib_global.h"
#include "Emulators/Core/EmulatorComponents.h"

#include <QString>

enum class typeSub {
    Trial,
    Arena,
    ArenaClan, // арена + клановые приколы
    Sobor,
    SoborClan, // арена + собор + клан
    Dungeon,
    Full,
    admin,
    UNKNOWN,
};

class QTextEdit;
class QWidget;

struct BOTLIB_EXPORT userProfile {
    int bot_ID = 0; // в GUI
    int user_ID; // account id
    int history_power;
    int count_units;
    bool state_premium, state_ads;
    QString leftover_time; // оставшееся время подписки
    QString emulator_name; // имя окна эмулятора
    typeSub subscribe;
    typeEmu emulatorType; // в GUI по идее
    QTextEdit *logger; // = new в GUI делать

    userProfile();
    userProfile(const userProfile &other);

    userProfile& operator=(const userProfile &other);

    QString getEmulatorType() const;

    QString getSubscribeType() const;

    void getInfo(QWidget *widget);
};


#endif // USERPROFILE_H
