#include "Cathedral/Cathedral.h"
#include "Cathedral/CathedralSettings.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

void Cathedral::checkPower(const cv::Mat &object, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    cv::Mat l_object;
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->changeColor(object,&l_object,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int power = 0;
    emit controller->Recognize(l_object,power);
    if(power <= 0) {
        observer.value.warning = m_Warning::FAIL_RECOGNIZE;
        controller->clickButton("battle/dark","button_best",&l_result);
        return;
    }
    if(power > settings->history_power) {
        controller->clickSwipe({468,194,0,0},{467,394,0,0},&l_result);
        if(!l_result) {
            observer.value.warning = m_Warning::MORE_THAN_HISTORY_POWER;
            observer.comment = QString::number(power) + " vs " + QString::number(settings->history_power);
            return;
        }
    }
    return;
}

void Cathedral::checkStage(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x < 3) {
        controller->checkLoading();

        controller->compareSample("dark/waypoints","sample_complete","compare_complete",&l_result,true,0.01);
        if(l_result) controller->clickButton("dark/waypoints","button_close");

        controller->compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
        if(l_result) {
            controller->clickButton("dark/waypoints/blessing","button_" + QString::number(1 + rand() % 3));
            controller->clickButton("dark/waypoints/blessing","button_confirm");
        }

        controller->Screenshot();
        for(int i = 0; i < 3; i++) {
            controller->compareSample("dark/waypoints","stage_" + QString::number(i+1),
                                      "compare_stage",&l_result,false,0.03);
            if(l_result) {
                currentStage = i+1;
                return;
            }
        }
        x++;
    }
    observer.value = l_result;
    observer.comment = "Cathderal floor";
    return;
}

void Cathedral::checkWarnings() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x < 6) {
        controller->compareSample("warnings/dark","sample_1","compare",&l_result,true,0.01);
        if(!l_result){
            controller->compareSample("warnings/dark","sample_2","compare",&l_result,false,0.01);
            if(!l_result) {
                x++;
                QThread::msleep(200);
                continue;
            }
        }
        controller->clickButton("warnings/dark","button_yes");
        return;
    }
}

void Cathedral::checkEndStage(ErrorList *result){
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->compareSample("dark/waypoints","sample_next","state_next",&l_result,true);
    if(l_result){
        controller->clickButton("dark/waypoints","button_next",&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        checkWarnings();
        //change on checkloading by controller ??
        do {
            controller->compareSample("load","sample","compare",&l_result,true);
            QThread::msleep(250);
        }
        while(!l_result);
        controller->checkLoading();
        l_result.warning = m_Warning::UNKNOWN;
        observer.value = l_result;
        return;
    }
    controller->compareSample("dark/waypoints","sample_end","state_end",&l_result);
    if(l_result) {
        controller->clickButton("dark/waypoints","button_end",&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        checkWarnings();
        currentStage = 4;
        return;
    }
    observer.value = l_result;
    observer.print = "No conditions to end stage";
    return;
}

void Cathedral::checkWaypoints(int &type, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while (x < 10){
        controller->compareSample("battle/dark","sample","compare",&l_result,true); // для баттла 0.04? вроде как 0.006 хватает
        if(l_result) {
            type = 4;
            return;
        }
        for(int i = 0; i < 4; i++){
            controller->compareSample("dark/waypoints/" + nameWaypoints.at(i),"sample","compare",&l_result); // 0.065??
            if(l_result){
                type = i;
                return;
            }
        }
        QThread::msleep(200);
        x++;
    }
    observer.value = l_result;
    observer.comment = "check waypoint";
    type = -1;
    return;
}

void Cathedral::checkBattleResult(bool *battle){
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        controller->compareSample("battle/end","sample","compare",&l_result,true,0.025);
        QThread::msleep(250);
    }
    while (!l_result);
    do {
        controller->compareSample("battle/end","sample_victory","state_victory",&l_result,true);
        if(!l_result) {
            controller->compareSample("battle/end","sample_defeat","state_victory",&l_result,false);
            if(l_result) {
                if(battle) *battle = false;
            }
        }
        else {
            if(battle) *battle = true;
        }
        controller->clickEsc();
        QThread::msleep(250);
        controller->compareSample("battle/end","sample","compare",&l_result,true,0.025); // ??
    } while(l_result);
}
