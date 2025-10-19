#ifndef WARNING_H
#define WARNING_H

enum class m_Warning {
    NO_WARN,
    UNKNOWN,
    //game
    FAIL_CHECK,
    FAIL_COMPARE,
    FAIL_PAGE,
    FAIL_CLICK,
    FAIL_RECOGNIZE,
    //emulator
    WRONG_EMULATOR_NAME,
    WRONG_EMULATOR_POS,
    WRONG_EMULATOR_SIZE,
    //other
    MORE_THAN_HISTORY_POWER,
    NO_EVENT,
};

#endif // WARNING_H
