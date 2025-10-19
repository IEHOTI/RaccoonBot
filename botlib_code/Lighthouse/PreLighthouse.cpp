#include "Lighthouse/Lighthouse.h"
#include "Lighthouse/LighthouseSettings.h"
#include "CustomError/ErrorObserver.h"
#include "Controller/Controller.h"

#include <QThread>

void Lighthouse::setUnitsSet(ErrorList *result) {
    controller->LocalLogging("======sets======");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->checkMainPage(&l_result);
    if(l_result.error == m_Error::FAIL_INIT) {
        controller->refreshMainPage(&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
    }
    controller->findBarracks(&l_result);
    if(!l_result) NoPrintError(&observer,l_result);
    controller->LocalLogging("======sets for cycle======");
    for(int i = 0; i < settings->squadSet.size();i++){
        controller->LocalLogging("======unit " + QString::number(i+1) + "======");
        controller->setUnitSet(i,settings->squadSet[i],&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
        QThread::msleep(500);
    }
    QThread::msleep(500);
    controller->LocalLogging("======check power======");
    controller->findBarracks(&l_result);
    if(!l_result) {
        controller->refreshMainPage(&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
    }
    else controller->entryBarracks(&l_result);

    if(!l_result) NoPrintError(&observer,l_result);

    controller->Screenshot();
    controller->setMask("squad/main/power");
    controller->findObject();
    checkPower(controller->cutImage(),&l_result);
    if(l_result.warning == m_Warning::MORE_THAN_HISTORY_POWER) saveHistoryPower(&l_result); // доработать
    else if(!l_result) NoPrintError(&observer,l_result);
    controller->clickEsc();
    QThread::msleep(500);
    controller->clickEsc();
}

void Lighthouse::checkPower(const cv::Mat &object, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if(object.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "empty object";
        return;
    }
    cv::Mat temp;
    controller->LocalLogging("======check power->change color======");
    controller->changeColor(object,&temp,&l_result);
    if(!l_result) NoPrintError(&observer,l_result);
    int number = -1;
    controller->LocalLogging("======check power->recognize======");
    emit controller->Recognize(temp, number);
    if(number < 0) {
        observer.value.warning = m_Warning::FAIL_RECOGNIZE;
        observer.comment = "lighthouse checkpower -> number < 0";
        return;
    }
    else if(number > settings->history_power) {
        observer.value.warning = m_Warning::MORE_THAN_HISTORY_POWER;
        return;
    }
    settings->user_power = number;
}
