#include "Arena/Arena.h"
#include "Arena/ArenaSettings.h"
#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"
#include "Controller/Controller.h"

#include <QThread>

Arena::Arena(Controller *g_controller, QObject *parent)
    : Task{parent}
{
    settings = nullptr;
    controller = g_controller;
    localPath = g_controller->getMainPath();
    maxPower = INT_MAX;
    currentStage = 0;
    stop_flag = false;
}

Arena::~Arena() {
    listPlayers.clear();
    blackList.clear();
    whiteList.clear();
    settings = nullptr;
    controller->Logging("Остановка задания [Арена].");
    controller->CleanUp();
    controller = nullptr;
}

void Arena::Initialize(TaskSettings *setting, ErrorList *result) {
    emit controller->Logging("Инициализация настроек арены.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    if (!setting) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "nullptr settings";
        return;
    }
    if(auto* arena = dynamic_cast<ArenaSettings*>(setting)) settings = arena;
    else {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "different type arena and unknown";
        emit controller->Logging("Произошла ошибка. Отправьте отчёт об ошибке.");
        settings = nullptr;
    }
    return;
}

void Arena::Pause() {
    throw PauseException();
}

void Arena::Stop(){
    throw StopException();
}

void Arena::Start(ErrorList *result) {
    emit controller->Logging("Задание [Арена] начато.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    try{
        setUnitsSet(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        controller->checkMainPage(&l_result);
        if(!l_result){
            controller->fixGameError(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
        }
        controller->clickButton("main","button_map");
        controller->checkMap(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        controller->clickMapButton("sample","button_arena",&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        int k = 0;
        stop_flag = false;
        while(!stop_flag){
            listPlayers.clear();
            checkStage(&l_result);
            if(!l_result) {
                observer.value = l_result;
                observer.print = false;
                return;
            }
            if(currentStage == 0){
                checkSettings(&l_result);
                if(l_result){
                    controller->clickButton("arena/main","button_start");
                    confirmSquad(&l_result);
                    if(!l_result) {
                        observer.value = l_result;
                        observer.print = false;
                        return;
                    }
                    waitFind();
                }
                else{
                    observer.value.error = m_Error::FAIL_INIT;
                    observer.comment = "arena settings";
                    emit controller->Logging("Задание прервано. Ошибка в настройках");
                    return;
                }
            }
            scanPlayers(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            printPlayers();
            attackPosition(settings->strategy.getPosition(listPlayers,currentStage,maxPower),&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            int tempCount = 0;
            if(currentStage < 5)
                do {
                    controller->compareSample("arena/battles","sample_next","state_wait",&l_result,true);
                    if(!l_result) {
                        tempCount++;
                        QThread::msleep(100);
                        if(tempCount == 10) {tempCount = 0; break;}
                    }
                    else QThread::msleep(500);
                } while(true);
            else
                do {
                    controller->compareSample("arena/battles","sample_end","state_wait",&l_result,true);
                    if(l_result) {
                        k++;
                        currentStage = 0;
                        controller->clickButton("arena/battles","button_home");
                        if(k == settings->count) stop_flag = true;
                        break;
                    }
                    else QThread::msleep(500);
                } while(true);
        }
        do controller->compareSample("arena/main","sample","compare",&l_result,true);
        while(!l_result);
        controller->clickButton("arena/main","button_close");
        controller->checkMainPage(&l_result);
        if(!l_result){
            controller->checkMap(&l_result);
            if(!l_result) {
                observer.value = l_result;
                observer.comment = "unknown page";
                return;
            }
            controller->clickButton("map","button_close");
        }
    } catch (const StopException &e) {
        observer.value.error = m_Error::STOP_TASK;
        observer.comment = e.what();
        QThread::currentThread()->quit();
        return;
    } catch (const PauseException &e){
        observer.value.error = m_Error::PAUSE_TASK;
        observer.comment = e.what();
        return;
    }
}
