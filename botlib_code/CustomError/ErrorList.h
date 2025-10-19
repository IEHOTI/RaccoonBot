#ifndef ERRORLIST_H
#define ERRORLIST_H
#include "BotLib_global.h"

#include "CustomError/Warning.h"
#include "CustomError/Error.h"
#include "qcontainerfwd.h"

struct BOTLIB_EXPORT ErrorList {
    m_Warning warning;
    m_Error error;

    bool operator!() const;
    explicit operator bool() const;
    static QString getError(m_Warning warning);
    static QString getError(m_Error error);
};

#endif // ERRORLIST_H
