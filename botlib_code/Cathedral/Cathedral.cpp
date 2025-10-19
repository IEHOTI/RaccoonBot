#include "Cathedral/Cathedral.h"
#include "Cathedral/CathedralSettings.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>
#include <QImage>

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
    if (currentStage <= 0 && currentStage >= 4) return;
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
            controller->clickButton("dark/waypoints/blessing","button" + QString::number(1 + rand() % 3));
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

void Cathedral::findWaypoint(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x<5) {
        QThread::msleep(200);
        controller->Screenshot();
        controller->setSample(controller->getMatObject(),&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        for(int i = 0; i < 7; i++) {
            cv::Mat l_object;
            controller->setMatObject("dark/map/sample_" + QString::number(i),&l_result);
            if(!l_result) NoPrintError(&observer,l_result);

            controller->setMask("dark/map/find_" + QString::number(i));
            controller->findObject(nullptr,&l_result);
            if(!l_result) NoPrintError(&observer,l_result);
            cv::Mat temp = controller->cutImage();
            controller->compareObject(0.12,&temp,nullptr,&l_result);//0.12?
            if(l_result) return;
        }
        x++;
    }
    checkEndStage(&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
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
        break;
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
            if(!l_result) {
                observer.value = l_result;
                observer.print = false;
                return;
            }
            controller->findObject();
            checkPower(controller->cutImage(),&l_result);
            if(!l_result) {
                x++;
                QThread::msleep(500);
            }
            else x = 3;
        } while(x < 3);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        x = 0;
        if(settings->premiumStatus){
            do{
                controller->clickButton("battle/dark","button_qstart",&l_result);
                if(l_result) break;
            }while(true);
            checkWarnings();
            checkBattleResult(battle);
            // while(x < 30){
            //     controller->compareSample("battle/end","sample","compare",&l_result,true,0.025);
            //     if(!l_result) break;
            //     else {
            //         x++;
            //         QThread::msleep(1000);
            //     }
            // }
            // if(x == 30){
            //     observer.value = l_result;
            //     observer.comment = "end battle";
            //     return;
            // }
            // controller->clickEsc();
            // ////?? надо ли, по-моему убрали их после битвы
            // controller->checkEvent(&l_result);
            // if(l_result) controller->skipEvent();
            // ////

        }
        else{
            //тут пока что пусто, лень
        }
        x = 0;
        while(x < 3){
            controller->compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
            if(l_result) break;
            x++;
            QThread::msleep(500);
        }
        if(l_result) {
            controller->clickButton("dark/waypoints/blessing","button_" + QString::number(1 + rand() % 3));
            controller->clickButton("dark/waypoints/blessing","button_confirm");
        }
        else {
            observer.value = l_result;
            observer.comment = "blessing";
        }
        return;
    }
    default:{
        observer.value.warning = m_Warning::FAIL_CHECK;
        observer.comment = "wrong type of waypoint";
        return;
    }
    }
    do controller->compareSample("dark/waypoints","sample_complete","compare_complete",&l_result,true);
    while(!l_result);//подумать на будущее над счетчиком и если что проверку на ошибки выбрасывать
    controller->clickButton("dark/waypoints","button_close");
    return;
}

void Cathedral::fullGamePass(ErrorList *result) {
    emit controller->Logging("Начато полное прохождение собора");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.");
        while(true) {
            findWaypoint(&l_result);
            if(!l_result) break;
            controller->click(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            int type = -1;
            checkWaypoints(type,&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            attackWaypoints(type,&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
        }
        if(currentStage == 4) break;
        checkStage(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
    }
}

void Cathedral::bossGamePass(ErrorList *result) {
    emit controller->Logging("Начато быстрое прохождение собора");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.");
        controller->Screenshot();
        controller->setSample(controller->getMatObject(),&l_result);

        controller->setMatObject("dark/map/sample_0",&l_result);
        if(!l_result) NoPrintError(&observer,l_result);

        controller->setMask("dark/map/find_0");
        controller->findObject(nullptr,&l_result);
        if(!l_result) NoPrintError(&observer,l_result);

        cv::Mat tempMat = controller->cutImage();
        controller->compareObject(0.12,&tempMat,nullptr,&l_result);
        QVector<cv::Rect> bossWay = getBossWay(controller->getRect());
        int n = bossWay.count();
        emit controller->errorLogging("==Начальная точка: " + QString::number(controller->getRect().x)
                                      + ";" + QString::number(controller->getRect().y));
        if(n > 1) {
            int errorCount = 0;
            int type = -1;
            for(int i = 0; i < n; i++){
                bool battle = true;
                controller->clickPosition(bossWay[i]);
                checkWaypoints(type,&l_result);
                if(!l_result){
                    i--;
                    errorCount++;
                    if(errorCount == 3) n = -5;
                    continue;
                }
                else errorCount = 0;
                attackWaypoints(type,nullptr,&battle);
                if(!battle) i--;//если поражение снова в ту точку
            }
            checkEndStage();
        }
        else{
            emit controller->Logging("Этаж был начат. Полное прохождение текущего этажа.");
            while(true) {
                findWaypoint(&l_result);
                if(!l_result) break;
                emit controller->errorLogging("Обнаружил точку на координатах: " + QString::number(controller->getRect().x)
                                              + ";" + QString::number(controller->getRect().y));
                controller->click(&l_result);
                if(!l_result){
                    observer.value = l_result;
                    observer.print = false;
                    return;
                }
                int type = -1;
                checkWaypoints(type,&l_result);
                if(!l_result){
                    observer.value = l_result;
                    observer.print = false;
                    return;
                }
                attackWaypoints(type,&l_result);
            }
        }
        if(currentStage == 4) break;
        checkStage(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
    }
}

void Cathedral::checkBattleResult(bool *battle){
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do controller->compareSample("battle/end","sample","compare",&l_result,true,0.025);
    while (!l_result);
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
    controller->clickEsc(nullptr,2);
}


