#include "CustomError/ErrorList.h"
#include <QString>

bool ErrorList::operator!() const {
    return warning != m_Warning::NO_WARN || error != m_Error::NO_ERR;
}

ErrorList::operator bool() const {
    return warning == m_Warning::NO_WARN && error == m_Error::NO_ERR;
}

QString ErrorList::getError(m_Warning warning) {
    switch (warning) {
    case m_Warning::UNKNOWN: return "Warning: UNKNOWN";
    case m_Warning::FAIL_CHECK: return "Warning: FAIL_CHECK";
    case m_Warning::FAIL_COMPARE: return "Warning: FAIL_COMPARE";
    case m_Warning::FAIL_PAGE: return "Warning: FAIL_PAGE";
    case m_Warning::FAIL_CLICK: return "Warning: FAIL_CLICK";
    case m_Warning::FAIL_RECOGNIZE: return "Warning: FAIL_RECOGNIZE";
    case m_Warning::WRONG_EMULATOR_NAME: return "Warning: WRONG_EMULATOR_NAME";
    case m_Warning::WRONG_EMULATOR_POS: return "Warning: WRONG_EMULATOR_POS";
    case m_Warning::WRONG_EMULATOR_SIZE: return "Warning: WRONG_EMULATOR_SIZE";
    case m_Warning::MORE_THAN_HISTORY_POWER: return "Warning: MORE_THAN_HISTORY_POWER";
    case m_Warning::NO_EVENT: return "Warning: NO_EVENT";
    default: return {};
    }
}

QString ErrorList::getError(m_Error error) {
    switch (error) {
    case m_Error::UNKNOWN: return "Error: UNKNOWN";
    case m_Error::EMPTY_IMG: return "Error: EMPTY_IMG";
    case m_Error::WRONG_IMG_PATH: return "Error: WRONG_IMG_PATH";
    case m_Error::FAIL_INIT: return "Error: FAIL_INIT";
    case m_Error::NO_ACTIVE_EMULATOR: return "Error: NO_ACTIVE_EMULATOR";
    default: return {};
    }
}
