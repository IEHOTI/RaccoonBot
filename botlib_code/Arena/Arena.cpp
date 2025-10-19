#include "Arena/Arena.h"
#include "Arena/ArenaSettings.h"
#include "CustomError/ErrorObserver.h"
#include "Controller/Controller.h"

#include <QThread>

void Arena::waitFind(){
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    do{
        controller->compareSample("arena/find","sample","compare",&l_result,true);
        if(!l_result) x++;
        else controller->click();
        QThread::msleep(1000);
    } while(x < 3);
    checkStage();
}

void Arena::checkStage(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->checkLoading();
    controller->compareSample("arena/battles","sample","compare",&l_result,true);
    if(!l_result){
        int count = 0;
        do{
            controller->compareSample("arena/main","sample","compare",&l_result,true);
            if(!l_result) {
                count++;
                QThread::msleep(1000);
            }
            else {
                currentStage = 0;
                return;
            }
        }while (count < 5);
        observer.value.warning = m_Warning::FAIL_COMPARE;
        observer.comment = "unknown page, not arena main or battles";
        return;
    }
    else{
        int count = 0;
        do {
            controller->compareSample("arena/battles","sample_end","state_wait",&l_result,true);
            if(l_result) {
                currentStage = 0;
                controller->clickButton("arena/battles","button_home");
                QThread::msleep(3000); // потом проверить мб 5к поставить
                return;
            }
            QThread::msleep(1000);
            count++;
        } while (count < 3);
    }
    controller->setMask("arena/battles/state_stage");
    controller->findObject();
    int temp = -1;
    emit controller->Recognize(controller->cutImage(),temp);
    temp /= 100;
    if((temp < 1) || (temp > 5)){
        observer.value.warning = m_Warning::FAIL_RECOGNIZE;
        observer.comment = "stage:" + QString::number(temp);
        return;
    }
    currentStage = temp;
}

void Arena::scanPlayers(ErrorList *result){
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->compareSample("arena/battles","sample_me","compare_me",nullptr,true,0.12);
    cv::Rect temp = controller->getRect();
    //scan my pos
    if((temp.y <= 215) && ((temp.y + temp.height) >= 215)) me.place = 1;
    else if((temp.y <= 320) && ((temp.y + temp.height) >= 320)) {
        if((temp.x <= 360) && ((temp.x + temp.width) >= 360)) me.place = 2;
        else if((temp.x <= 540) && ((temp.x + temp.width) >= 540)) me.place = 3;
        else me.place = 0;
    }
    else if ((temp.y <= 425) && ((temp.y + temp.height) >= 425)){
        if((temp.x <= 270) && ((temp.x + temp.width) >= 270)) me.place = 4;
        else if((temp.x <= 450) && ((temp.x + temp.width) >= 450)) me.place = 5;
        else if((temp.x <= 630) && ((temp.x + temp.width) >= 630)) me.place = 6;
        else me.place = 0;
    }
    else if((temp.y <= 530) && ((temp.y + temp.height) >= 530)){
        if((temp.x <= 180) && ((temp.x + temp.width) >= 180)) me.place = 7;
        else if((temp.x <= 360) && ((temp.x + temp.width) >= 360)) me.place = 8;
        else if((temp.x <= 540) && ((temp.x + temp.width) >= 540)) me.place = 9;
        else if((temp.x <= 720) && ((temp.x + temp.width) >= 720)) me.place = 10;
        else me.place = 0;
    }
    else me.place = 0;
    if(me.place == 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "my place 0";
        return;
    }
    if(me.place != 1) controller->clickButton("arena/battles","button_1");
    else controller->clickButton("arena/battles","button_2");
    do {
        controller->compareSample("battle/arena","sample","compare",&l_result,true);
        QThread::msleep(1000);
    } while(!l_result);
    controller->setMask("battle/arena/my_power");
    controller->findObject();
    emit controller->Recognize(controller->cutImage(),me.power);
    me.status = State::IS_ME;
    for(int i = 1; i <= 10; i++){
        if(me.place == i) {
            listPlayers.append(me);
            continue;
        }
        ArenaPlayer tempPlayer;
        controller->setMask("battle/arena/enemy_power");
        controller->findObject();
        emit controller->Recognize(controller->cutImage(),tempPlayer.power);
        tempPlayer.place = i;
        //black and whiteList later
        controller->compareSample("battle/arena","sample_available","state_battle",&l_result,true);
        if(l_result) tempPlayer.status = State::AVAILABLE;
        else {
            controller->compareSample("battle/arena","sample_victory","state_battle",&l_result);
            if(l_result) tempPlayer.status = State::VICTORY;
            else {
                controller->compareSample("battle/arena","sample_defeat","state_battle",&l_result);
                if(l_result) tempPlayer.status = State::DEFEAT;
                else tempPlayer.status = State::UNKNOWN;
            }
        }
        listPlayers.append(tempPlayer);
        do {
            controller->clickButton("battle/arena","button_next",&l_result);
            QThread::msleep(1250); // 1000->1250
        } while(!l_result);
    }
    controller->clickEsc();
}

void Arena::attackPosition(int pos, ErrorList *result){
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if(listPlayers.size() != 10) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "scan player: " + QString::number(listPlayers.size()) + " of 10";
        return;
    }
    controller->clickButton("arena/battles","button_" + QString::number(pos));
    do {
        controller->compareSample("battle/arena","sample","compare",&l_result,true);
        if(!l_result) QThread::msleep(1000);
    } while (!l_result);


    ///bl wl
    //controller->setMask("battle/arena/enemy_name");
    //controller->findObject();
    //Mat tempMat = controller->cutImage();
    //ArenaPlayer temp = listPlayers[pos-1];
    ///

    if(settings->premiumStatus) {
        controller->clickButton("battle/arena","button_qstart");
        bool battle = false;
        checkBattleResult(&battle);
    }
    else controller->clickButton("battle/arena","button_start"); // dodelat

    ///bl wl
    //if((temp.power > maxPower) && battle) ;//save whitelist
    //if(temp.blackList && battle) ;//save whitelist delete blacklist
    //if(temp.whiteList && !battle) ; //save bl delete wl
    //if(!battle) ; //save bl
    ///
}

void Arena::printPlayers(){
    emit controller->Logging("Текущий этап: " + QString::number(currentStage));
    for(int i = 0,n = listPlayers.size(); i < n; i++) {
        ArenaPlayer temp = listPlayers[i];
        QString str = "[" + QString::number(i) + "] Место: " + QString::number(temp.place);
        str += " Мощь: " + QString::number(temp.power) + "\n";
        str += QString(temp.blackList ? "BL" : "")
            +  QString(temp.whiteList ? "WL" : "");
        str += " Статус: ";
        switch (temp.status){
        case (State::AVAILABLE): {
            str += "Атаковать";
            break;
        }
        case (State::DEFEAT): {
            str += "Поражение";
            break;
        }
        case (State::VICTORY): {
            str += "Победа";
            break;
        }
        case (State::IS_ME): {
            str += "Это я";
            break;
        }
        case (State::UNKNOWN): {
            str += "Неизвестно, ошибка";
            break;
        }
        default: {
            str += "??";
            break;
        }
        }
        emit controller->Logging(str,false);
    }
}

void Arena::checkBattleResult(bool *battle) {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        //controller->skipEvent(); а нужен ли
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
    QThread::msleep(500);
    controller->checkEvent(&l_result);
    if(l_result) controller->skipEvent();
    controller->clickEsc(nullptr,2);
}
