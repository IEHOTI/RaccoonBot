#include "Cathedral/Cathedral.h"
#include "Cathedral/CathedralSettings.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

void Cathedral::findWaypoint(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x < 5) {
        QThread::msleep(250);
        controller->Screenshot();
        controller->setSample(controller->getMatObject(),&l_result);
        if(!l_result) NoPrintError(&observer, l_result);

        for(int i = 0; i < 7; ++i) {
            cv::Mat l_object;
            controller->setMatObject("dark/map/sample_" + QString::number(i),&l_result);
            if(!l_result) NoPrintError(&observer,l_result);

            controller->setMask("dark/map/find_" + QString::number(i));
            if(!l_result) NoPrintError(&observer,l_result);

            controller->findObject(nullptr,&l_result);
            if(!l_result) NoPrintError(&observer,l_result);

            cv::Mat temp = controller->cutImage();
            controller->compareObject(0.12,&temp,nullptr,&l_result);//0.12?
            if(l_result) return;
        }
        ++x;
    }
    checkEndStage(&l_result);
}


void Cathedral::attackWaypoints(int type, ErrorList *result, bool *battle) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    switch (type) {
    case 0: {//altar
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_move");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 1: {//camp
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_reward");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        checkWarnings();
        break;
    }
    case 2: {//game
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_skip");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        return;
    }
    case 3: {//reflection
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 4: {//battle
        int x = 0;
        do {
            controller->Screenshot();
            controller->setMask("battle/dark/state_power",&l_result);
            if(!l_result) NoPrintError(&observer,l_result);

            controller->findObject();
            checkPower(controller->cutImage(),&l_result);
            if(!l_result) {
                if (++x > 3) NoPrintError(&observer,l_result);
                else QThread::msleep(500);
            }
            else break;
        } while(true);

        if(settings->premiumStatus){
            do{
                controller->clickButton("battle/dark","button_qstart",&l_result);
                if(l_result) break;
            } while(true);
            checkWarnings();
            checkBattleResult(battle);
        }
        else{
            //тут пока что пусто, лень
        }
        do {
            controller->compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
            if(l_result) {
                controller->clickButton("dark/waypoints/blessing","button_" + QString::number(1 + rand() % 3));
                QThread::msleep(50);
                controller->clickButton("dark/waypoints/blessing","button_confirm");
                QThread::msleep(500);
            }
        } while(l_result);
        return;
    }
    default:{
        observer.value.warning = m_Warning::FAIL_CHECK;
        observer.comment = "wrong type of waypoint";
        return;
    }

    }
    int x = 0;
    do {
        controller->compareSample("dark/waypoints","sample_complete","compare_complete",&l_result,true);
        if(l_result) break;
        else if(++x > 15) controller->fixErrors();
        else QThread::msleep(500);
    }
    while(!l_result);
    controller->clickButton("dark/waypoints","button_close");
    return;
}
