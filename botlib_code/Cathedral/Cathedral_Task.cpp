#include "Cathedral/Cathedral.h"
#include "Cathedral/CathedralSettings.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"

#include <QThread>

Cathedral::Cathedral(Controller *g_controller, QObject* parent)  : Task(parent) {
    settings = nullptr;
    controller = g_controller;
    localPath = g_controller->getMainPath();
    nameWaypoints << "altar" << "camp" << "game" << "reflection";
    currentStage = 0;
    stop_flag = false;
}

Cathedral::~Cathedral() {
    settings = nullptr;
    controller->Logging("Остановка задания [Собор].");
    controller->CleanUp();
    controller = nullptr;
}

void Cathedral::Initialize(TaskSettings *setting, ErrorList *result) {
    emit controller->Logging("Инициализация настроек собора.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    if (!setting) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "settings nullptr";
        return;
    }
    if(auto* cathedral = dynamic_cast<CathedralSettings*>(setting)) settings = cathedral;
    else {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "wrong type: cathedral -> unknown";
        emit controller->Logging("Произошла ошибка. Отправьте отчёт об ошибке.");
        settings = nullptr;
    }
    return;
}
void Cathedral::Pause() {
    throw PauseException();
}

void Cathedral::Stop() {
    throw StopException();
}

void Cathedral::Start(ErrorList *result) {
    emit controller->Logging("Задание [Собор] начато.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    try {
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
        controller->clickMapButton("sample","button_dark",&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        int k = 0;
        stop_flag = false;
        while(!stop_flag) {
            currentStage = 0;
            checkMain(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = "check_main";
                return;
            }
            if(currentStage == 0) {
                checkSettings(&l_result);
                if(l_result) {
                    controller->clickButton("dark","button_start");
                    confirmSquad(&l_result);
                    if(!l_result){
                        observer.value = l_result;
                        observer.print = false;
                        return;
                    }
                }
                else {
                    observer.value = l_result;
                    observer.print = false;
                    emit controller->Logging("Задание прервано. Ошибка в настройках");
                    return;
                }
                checkStage(&l_result);
                if(!l_result){
                    observer.value = l_result;
                    observer.print = false;
                    return;
                }
            }
            if(settings->fullGamePass) fullGamePass(&l_result);
            else bossGamePass(&l_result);
            do controller->compareSample("dark","sample_end","compare_end",&l_result,true);
            while(!l_result);
            do {
                controller->compareSample("load","sample","compare",&l_result,true);
                if(!l_result) {
                    controller->clickButton("dark","button_end");
                    QThread::msleep(200);
                }
            } while(!l_result);
            controller->checkLoading();
            QThread::msleep(750);
            controller->checkEvent(&l_result);
            if(l_result) controller->skipEvent();
            k++;
            if(k == settings->count) stop_flag = true;
        }
        controller->clickButton("dark","button_close");
        controller->checkMainPage(&l_result);
        if(!l_result){
            controller->checkMap(&l_result);
            if(!l_result) {
                observer.value = l_result;
                observer.print = false;
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
