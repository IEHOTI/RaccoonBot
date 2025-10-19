#ifndef ERROROBSERVER_H
#define ERROROBSERVER_H

#include <QObject>
#include "BotLib_global.h"
#include "ErrorList.h"

class BOTLIB_EXPORT ErrorObserver : public QObject {
    Q_OBJECT
public:
    ErrorList *ptr = nullptr;
    ErrorList value = {m_Warning::NO_WARN,m_Error::NO_ERR};
    QString comment = "";
    bool print = true;

    explicit ErrorObserver(ErrorList *result = nullptr, QObject *parent = nullptr);
    ~ErrorObserver();
signals:
    void Logging(const QString &msg);
};

inline void NoPrintError(ErrorObserver *observer, ErrorList result) {
    observer->value = result;
    observer->print = false;
    return;
}

#endif // ERROROBSERVER_H
