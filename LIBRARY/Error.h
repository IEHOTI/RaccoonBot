#ifndef ERROR_H
#define ERROR_H
#include "BotLib_global.h"
#include <QDateTime>
struct ErrorList {
    enum class Warning {
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
    } warning;

    enum class Error {
        NO_ERR,
        UNKNOWN,
        //image
        EMPTY_IMG,
        WRONG_IMG_PATH,
        //emulator
        FAIL_INIT,
        NO_ACTIVE_EMULATOR,
        //other
        STOP_TASK,
        PAUSE_TASK,
    } error;

    bool operator!() const {
       return warning != Warning::NO_WARN || error != Error::NO_ERR;
    }
    explicit operator bool() const {
        return warning == Warning::NO_WARN && error == Error::NO_ERR;
    }

    static QString getError(Warning warning) {
        switch (warning) {
        case Warning::UNKNOWN: return "Warning: UNKNOWN";
        case Warning::FAIL_CHECK: return "Warning: FAIL_CHECK";
        case Warning::FAIL_COMPARE: return "Warning: FAIL_COMPARE";
        case Warning::FAIL_PAGE: return "Warning: FAIL_PAGE";
        case Warning::FAIL_CLICK: return "Warning: FAIL_CLICK";
        case Warning::FAIL_RECOGNIZE: return "Warning: FAIL_RECOGNIZE";
        case Warning::WRONG_EMULATOR_NAME: return "Warning: WRONG_EMULATOR_NAME";
        case Warning::WRONG_EMULATOR_POS: return "Warning: WRONG_EMULATOR_POS";
        case Warning::WRONG_EMULATOR_SIZE: return "Warning: WRONG_EMULATOR_SIZE";
        case Warning::MORE_THAN_HISTORY_POWER: return "Warning: MORE_THAN_HISTORY_POWER";
        case Warning::NO_EVENT: return "Warning: NO_EVENT";
        default: return {};
        }
    }

    static QString getError(Error error) {
        switch (error) {
        case Error::UNKNOWN: return "Error: UNKNOWN";
        case Error::EMPTY_IMG: return "Error: EMPTY_IMG";
        case Error::WRONG_IMG_PATH: return "Error: WRONG_IMG_PATH";
        case Error::FAIL_INIT: return "Error: FAIL_INIT";
        case Error::NO_ACTIVE_EMULATOR: return "Error: NO_ACTIVE_EMULATOR";
        default: return {};
        }
    }
};

using m_Warning = ErrorList::Warning;
using m_Error = ErrorList::Error;

class BOTLIB_EXPORT ErrorObserver : public QObject {
    Q_OBJECT
public:
    explicit ErrorObserver(ErrorList *result = nullptr, QObject *parent = nullptr): QObject(parent), ptr(result) {}
    ~ErrorObserver() {
        if(ptr) *ptr = value;
        if(!value && print) emit Logging("[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] "
                         + ErrorList::getError(value.warning) + " "
                         + ErrorList::getError(value.error) + " "
                         + comment);
    }
    ErrorList *ptr = nullptr;
    ErrorList value = {m_Warning::NO_WARN,m_Error::NO_ERR};
    QString comment = "";
    bool print = true;
signals:
    void Logging(const QString &msg);
};

inline void NoPrintError(ErrorObserver *observer, ErrorList result) {
    observer->value = result;
    observer->print = false;
    return;
}

#endif // ERROR_H
