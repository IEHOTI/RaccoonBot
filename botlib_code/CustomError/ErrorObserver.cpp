#include "CustomError/ErrorObserver.h"
#include <QDateTime>

ErrorObserver::ErrorObserver(ErrorList *result, QObject *parent)
    : QObject{parent}, ptr(result)
{}

ErrorObserver::~ErrorObserver() {
    if(ptr) *ptr = value;
    if(!value && print) emit Logging("[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] "
                     + ErrorList::getError(value.warning) + " "
                     + ErrorList::getError(value.error) + " "
                     + comment);
}

