#include "Lighthouse/Lighthouse.h"
#include "Lighthouse/LighthouseSettings.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

void Lighthouse::checkBattleResult(bool *battle) {
    controller->LocalLogging("======check battle result======");
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        controller->compareSample("battle/end","sample","compare",&l_result,true);
        if(l_result) break;
    }while(true);
    ///
    controller->compareSample("battle/end","sample_victory","state_victory",&l_result,true);
    if(l_result) {
        if (battle) *battle = true;
    }
    else {
        controller->compareSample("battle/end","sample_defeat","state_victory",&l_result,true);
        if(l_result) if(battle) *battle = false;
    }
    //
    controller->clickEsc();
    QThread::msleep(500);
    controller->checkLoading();
    controller->checkEvent(&l_result);
    if(l_result) controller->skipEvent();
}

void Lighthouse::scanEnemy(int &attack_pos, cv::Mat &resultScan, ErrorList *result){
    controller->LocalLogging("======scan enemy======");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->Screenshot();
    for(int i = 1; i < 6; i++) {
        controller->compareSample("lighthouse","sample","button_" + QString::number(i),&l_result);
        if(l_result){
            controller->setMask("lighthouse/power_" + QString::number(i));
            controller->findObject();
            int power = 0;
            controller->Recognize(controller->cutImage(),power);
            controller->LocalLogging("scan " + QString::number(i) + " unit - " + QString::number(power) + "user power: "
                                     + QString::number(settings->user_power) + ", rangePower: "
                                     + QString::number(settings->rangePower));
            if(power > settings->user_power * settings->rangePower) continue;
            attack_pos = i;
            controller->setMask("lighthouse/enemy_" + QString::number(i));
            controller->findObject();
            controller->cutImage().copyTo(resultScan);
            return;
        }
        if(i+1 == 6) {
            observer.value.warning = m_Warning::MORE_THAN_HISTORY_POWER;
            observer.print = false;
            return;
        }
    }
    observer.value.warning = m_Warning::FAIL_COMPARE;
    observer.print = false;
    return;
}

void Lighthouse::attackEnemy(int pos, int &count, ErrorList *result) {
    controller->LocalLogging("======attack enemy======");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->clickButton("lighthouse","button_" + QString::number(pos));
    int x = 0;
    do {
        controller->compareSample("battle/lighthouse","sample","compare",&l_result,true);
        if(!l_result){
            x++;
            if(x == 50) NoPrintError(&observer,l_result);
            QThread::msleep(1000);
        }
    } while(!l_result);

    if(settings->premiumStatus) controller->clickButton("battle/lighthouse","button_qstart");
    else controller->clickButton("battle/lighthouse","button_start");

    checkWarning();
    controller->compareSample("lighthouse","no_energy","compare_energy",&l_result,true);
    if(l_result) {
        if(count != -1) {
            count = INT_MAX;
            controller->clickEsc();
            controller->clickEsc();
            return;
        }
        controller->clickButton("lighthouse","button_energy");
    }

    if(!settings->premiumStatus) controller->fixBattleSettings();
}

void Lighthouse::refreshEnemy(int &count_attack,int count_lose, int &count_refresh, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->compareSample("lighthouse","sample_free_refresh","compare_refresh",&l_result,true);
    if(l_result) {
        bool refresh = false;
        switch (settings->modeRefresh) {
        case -1: {
            if(count_lose == 1) {
                controller->clickButton("lighthouse","button_refresh");
                refresh = true;
            }
            break;
        }
        case 3:{
            if(count_attack == 2) {
                controller->clickButton("lighthouse","button_refresh");
                refresh = true;
            }
            break;
        }
        case 5:{
            if(count_attack == 4) {
                controller->clickButton("lighthouse","button_refresh");
                refresh = true;
            }
            break;
        }
        default:{
            observer.value = l_result;
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "wrong modeRefresh setting";
            return;
        }
        }
        if((observer.value.warning == m_Warning::MORE_THAN_HISTORY_POWER) && !refresh) {
            controller->clickButton("lighthouse","button_refresh");
            refresh = true;
        }
        if(refresh) {
            count_refresh++;
            QThread::msleep(1500);
        }
    }
    else {
        controller->compareSample("lighthouse","sample_diamond_refresh","compare_refresh",&l_result,true);
        if(!l_result) {
            observer.value = l_result;
            observer.comment = "wrong refresh button";
            return;
        }
        if(count_refresh > settings->diamondRefresh) {
            if(observer.value.warning == m_Warning::MORE_THAN_HISTORY_POWER) count_attack = INT_MAX;
            return;
        }
        bool refresh = false;
        switch (settings->modeRefresh) {
        case -1: {
            if(count_lose == 1) {
                controller->clickButton("lighthouse","button_refresh");
                refresh = true;
            }
            break;
        }
        case 3:{
            if(count_attack == 2){
                controller->clickButton("lighthouse","button_refresh");
                refresh = true;
            }
            break;
        }
        case 5:{
            if(count_attack == 4) {
                controller->clickButton("lighthouse","button_refresh");
                refresh = true;
            }
            break;
        }
        default:{
            observer.value = l_result;
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "wrong modeRefresh setting";
            return;
        }
        }
        if((observer.value.warning == m_Warning::MORE_THAN_HISTORY_POWER) && !refresh) {
            controller->clickButton("lighthouse","button_refresh");
            refresh = true;
        }
        if(refresh) {
            count_refresh++;
            QThread::msleep(1500);
        }
    }
    observer.value = l_result;
}

void Lighthouse::checkWarning() {
    QThread::msleep(500);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->compareSample("warnings/lighthouse","sample","compare",&l_result,true);
    if(l_result) controller->clickButton("warnings/lighthouse","button_yes");
    QThread::msleep(500);
}


