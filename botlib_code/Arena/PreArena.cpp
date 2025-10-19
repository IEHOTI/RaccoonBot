#include "Arena/Arena.h"
#include "Arena/ArenaSettings.h"
#include "CustomError/ErrorObserver.h"
#include "Controller/Controller.h"

#include <QThread>

void Arena::confirmSquad(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        controller->compareSample("squad/arena","sample","compare",&l_result,true);
        if(!l_result) QThread::msleep(1000);
    } while(!l_result);
    switch (settings->modeSquad) {
    case 0: {
        controller->clickButton("squad/arena","button_best",&l_result,2);
        break;
    }
    case 1: {
        controller->clickButton("squad/arena","button_previous",&l_result,2);
        break;
    }
    case 2: {
        break;
    }
    default:{
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "wrong squad mode";
        return;
    }
    }
    //controller->setMask("squad/arena/power");
    //controller->findObject();
    //checkPower(controller->cutImage(),&l_result);
    /*if(l_result.warning == m_Warning::MORE_THAN_HISTORY_POWER){
        savePower(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
    } else if (!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    else */controller->clickButton("squad/arena","button_start");
}

void Arena::checkSettings(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    if(settings->history_power < 1) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "history power < 1";
        return;
    }
    switch(settings->modeTicket) {
    case 0:{
        controller->clickButton("arena/main","button_apples");
        break;
    }
    case 1:{
        controller->clickButton("arena/main","button_ticket");
        break;
    }
    case 2://{пока что не работает}
    case 3://туда же
    default:{
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "wrong key";
        return;
    }
    }

}

void Arena::checkPower(const cv::Mat &object, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if(object.empty()){
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "empty object";
        return;
    }
    cv::Mat temp;
    //imwrite("G:/Coding/Photo/ocr/before_change.png", object);
    controller->changeColor(object,&temp,&l_result);
    //imwrite("G:/Coding/Photo/ocr/after_change.png", object);
    //imwrite("G:/Coding/Photo/ocr/after_temp.png", temp);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int number = -1;
    emit controller->Recognize(temp,number);
    if(number < 0) {
        observer.value.warning = m_Warning::FAIL_RECOGNIZE;
        observer.comment = "arena checkPower -> number < 0";
        return;
    }
    else if(number > settings->history_power) {
        observer.value.warning = m_Warning::MORE_THAN_HISTORY_POWER;
        return;
    }
}

void Arena::savePower(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(!l_result || x < 3){
        controller->clickSwipe({460,180,0,0},{460,400,0,0},&l_result);
        controller->Screenshot();
        checkPower(controller->cutImage(),&l_result);
        if(l_result.warning == m_Warning::MORE_THAN_HISTORY_POWER) x++;
        else {
            observer.value = l_result;
            observer.comment = "fix";
            return;
        }
    }
    if(x == 3) {
        observer.value = l_result;
        observer.comment = "cant save power";
        return;
    }
    int y = 0;
    while(y < 10){
        controller->clickSwipe({770,330,0,0},{170,330,0,0});
        y++;
    }
    while(x > 0){
        controller->clickSwipe({420 + (x * 150) ,390,0,0},{420,180,0,0});
        x--;
    }
}

void Arena::setUnitsSet(ErrorList *result) {
    if(settings->modeSquad != 2) return;
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->checkMainPage(&l_result);
    if(l_result.error == m_Error::FAIL_INIT) {
        controller->refreshMainPage(&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
    }
    controller->findBarracks(&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    for(int i = 0; i < settings->squadSet.size();i++){
        controller->setUnitSet(i,settings->squadSet[i],&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        QThread::msleep(1000);
    }
}
