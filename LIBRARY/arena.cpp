#include "arena.h"

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
                        Sleep(100);
                        if(tempCount == 10) {tempCount = 0; break;}
                    }
                    else Sleep(500);
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
                    else Sleep(500);
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

void Arena::confirmSquad(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        controller->compareSample("squad/arena","sample","compare",&l_result,true);
        if(!l_result) Sleep(1000);
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
    case 2: {//сделать овер умный типа сначала бест нажать на поиск и сразу отмена а потом в казарме вешалки раскинуть
        break;
    }
    default:{
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "wrong squad mode";
        return;
    }
    }
    controller->setMask("squad/arena/power");
    controller->findObject();
    checkPower(controller->cutImage(),&l_result);
    if(l_result.warning == m_Warning::MORE_THAN_HISTORY_POWER){
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
    else controller->clickButton("squad/arena","button_start");
}

void Arena::waitFind(){
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    do{
        controller->compareSample("arena/find","sample","compare",&l_result,true);
        if(!l_result) x++;
        else controller->click();
        Sleep(1000);
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
                Sleep(1000);
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
                Sleep(3000); // потом проверить мб 5к поставить
                return;
            }
            Sleep(1000);
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
    Rect temp = controller->getRect();
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
    //emit controller->Logging("me x: " + QString::number(temp.x) + " y: " + QString::number(temp.y),false);
    //
    if(me.place == 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "my place 0";
        return;
    }
    if(me.place != 1) controller->clickButton("arena/battles","button_1");
    else controller->clickButton("arena/battles","button_2");
    do {
        controller->compareSample("battle/arena","sample","compare",&l_result,true);
        Sleep(1000);
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
            Sleep(1250); // 1000->1250
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
        if(!l_result) Sleep(1000);
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

void Arena::checkPower(const Mat &object, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if(object.empty()){
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "empty object";
        return;
    }
    Mat temp;
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
        observer.comment = "number < 0";
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
        controller->compareSample("battle/end/quick","sample","compare",&l_result,true);
        if(l_result) break;
    }while(true);
    ///
    controller->compareSample("battle/end/quick","sample_victory","state_victory",&l_result,true);
    if(l_result) {
        if (battle) *battle = true;
    }
    else {
        controller->compareSample("battle/end/quick","sample_defeat","state_victory",&l_result,true);
        if(l_result) if(battle) *battle = false;
    }
    //
    Sleep(500);
    controller->checkEvent(&l_result);
    if(l_result) controller->skipEvent();
    controller->clickEsc(nullptr,2);
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
    }
}
void Arena::savePlayerToBlackList(const Mat &player, int playerPower){}
void Arena::savePlayerToWhileList(const Mat &player, int playerPower){}
void Arena::loadLists(int playerID) {}
