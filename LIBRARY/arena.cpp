#include "arena.h"

Arena::Arena(Controller *g_controller, QObject *parent)
    : Task{parent}
{
    settings = nullptr;
    controller = g_controller;
    localPath = g_controller->getMainPath();
    maxPower = INT_MAX;
    currentStage = 0;
}

Arena::~Arena() {}

void Arena::Initialize(TaskSettings *setting, bool *result) {
    emit controller->Logging("Инициализация настроек арены.");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    if (!setting) {
        err.value = false;
        err.errorMessage = "Передан нулевой указатель на настройки.";
        return;
    }
    if(auto* arena = dynamic_cast<ArenaSettings*>(setting)) settings = arena;
    else {
        err.value = false;
        err.errorMessage = "Неправильный тип настроек подан на вход.";
        emit controller->Logging("Произошла ошибка. Отправьте отчёт об ошибке.");
        settings = nullptr;
    }
    return;
}
void Arena::Stop(){

}

void Arena::Start(bool *result) {
    emit controller->Logging("Задание [Арена] начато.");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    controller->checkMainPage(&l_result);
    if(!l_result){
        controller->fixGameError(&l_result);
        if(!l_result){
            err.value = false;
            err.errorMessage = "Фиг знает где мы. Вызывайте чинилыча.";
            return;
        }
    }
    controller->clickButton("main","button_map");
    controller->checkMap(&l_result);
    if(!l_result){
        err.value = false;
        err.errorMessage = "Карта не открылась.";
        return;
    }
    //
    controller->clickButton("map","button_arena");
    //
    int k = 0;
    stop_flag = false;
    while(!stop_flag){
        listPlayers.clear();
        checkStage();
        if(currentStage == 0){
            checkSettings(&l_result);
            if(l_result){
                controller->clickButton("arena/main","button_start");
                confirmSquad(&l_result);
                if(!l_result) {
                    err.value = false;
                    err.errorMessage = "Не удалось подтвердить отряд";
                    return;
                }
                waitFind();
            }
            else{
                err.value = false;
                err.errorMessage = "Настройки арены кривые...";
                controller->Logging("Задание прервано. Ошибка в настройках");
                return;
            }
        }
        //
        scanPlayers();
        printPlayers();
        attackPosition(settings->strategy.getPosition(listPlayers,currentStage,maxPower),&l_result);
        if(!l_result){
            err.value = false;
            err.errorMessage = "Не удалось тыкнуть во врага. Кол-во просканированных: " + QString::number(listPlayers.count());
            return;
        }
        //
        int tempCount = 0;
        if(currentStage < 5)
            do {
                controller->compareSample("arena/battles","sample_next","state_wait",&l_result,true);
                if(!l_result) {
                    tempCount++;
                    Sleep(150);
                    if(tempCount == 10) {tempCount = 0; break;}
                }
                else Sleep(1000);
            } while(true);//тут сделать checkWait в 2 строчках
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
                else Sleep(1000);
            } while(true);//сделать checkEnd там же сделать зануление стадии и к++
    }
    do controller->compareSample("arena/main","sample","compare",&l_result,true);
    while(!l_result);
    controller->clickButton("arena/main","button_close");
    controller->checkMainPage(&l_result);
    if(!l_result){
        controller->checkMap(&l_result);
        if(!l_result) {
            err.value = false;
            err.errorMessage = "Неизвестное местонахождение";
            return;
        }
        controller->clickButton("map","button_close");
    }
}

void Arena::confirmSquad(bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    while(!l_result) {
        controller->compareSample("squad/arena","sample","compare",&l_result,true);
        if(!l_result) Sleep(1000);
    }
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
        err.value = false;
        err.errorMessage = "Неправильная настройка.";
        return;
    }
    }
    controller->setMask("squad/arena/power");
    controller->findObject();
    checkPower(controller->cutImage(),&l_result);
    if(!l_result){
        savePower(&l_result);
        if(!l_result){
            err.value = false;
            err.errorMessage = "Невозможно начать арену. Выставляемая мощь больше чем ИМ";
            return;
        }
    }

    controller->clickButton("squad/arena","button_start");
}

void Arena::waitFind(){
    bool l_result = false;
    int x = 0;
    do{
        controller->compareSample("arena/find","sample","compare",&l_result,true);
        if(!l_result) x++;
        else controller->click();
        Sleep(1000);
    } while(x < 3);
    checkStage();
}

void Arena::checkStage(bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    controller->checkLoading();
    controller->compareSample("arena/battles","sample","compare",&l_result,true);
    if(!l_result){
        int count = 0;
        do{
            controller->compareSample("arena/main","sample","compare",&l_result);
            if(!l_result) {
                count++;
                Sleep(1000);
            }
            else break;
        }while (count < 5);
        if (count < 5){
            currentStage = 0;
            return;
        }
        else {
            err.value = false;
            err.errorMessage = "Ни арены, ни битв арены";
            return;
        }
    }
    //проверку на подарок, если есть забрать и стейдж = 0
    //
    controller->setMask("arena/battles/state_stage");
    controller->findObject();
    int temp = -1;
    emit controller->Recognize(controller->cutImage(),temp);
    temp /= 100;
    if((temp < 1) || (temp > 5)){
        err.value = false;
        err.errorMessage = "Неверный детект этапа арены: " + QString::number(temp);
        return;
    }
    currentStage = temp;
}

void Arena::scanPlayers(){
    bool l_result = false;
    controller->compareSample("arena/battles","sample_me","compare_me",nullptr,true,0.12);
    Rect temp = controller->getRect();
    l_result = false;
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
    emit controller->Logging("me x: " + QString::number(temp.x) + " y: " + QString::number(temp.y));
    //
    if(me.place == 0) return;
    if(me.place != 1) controller->clickButton("arena/battles","button_1");
    else controller->clickButton("arena/battles","button_2");
    l_result = false;
    while(!l_result) {
        controller->compareSample("battle/arena","sample","compare",&l_result,true);
        Sleep(1000);
    }
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
        l_result = false;
        while(!l_result) {
            controller->clickButton("battle/arena","button_next",&l_result);
            Sleep(1000);
        }
    }
    controller->clickEsc();
}

void Arena::attackPosition(int pos, bool *result){
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    if(listPlayers.size() != 10) {
        err.value = false;
        err.errorMessage = "Произошла ошибка на этапе скана игроков.";
        return;
    }
    controller->clickButton("arena/battles","button_" + QString::number(pos));
    l_result = false;
    while (!l_result) {
        controller->compareSample("battle/arena","sample","compare",&l_result,true);
        if(!l_result) Sleep(1000);
    }
    //вместо тру потом условие на прем у игрока.
    if(true) {
        ///bl wl
        controller->setMask("battle/arena/enemy_name");
        controller->findObject();
        Mat tempMat = controller->cutImage();
        ///
        controller->clickButton("battle/arena","button_qstart");
        int count = 0;
        do{
        controller->skipEvent();
        controller->compareSample("battle/end/quick","sample","compare",&l_result,true);
        if(!l_result) {
            count++;
            Sleep(1000);
        }
        else count = 5;
        }while(count < 5);
        ///
        controller->compareSample("battle/end/quick","sample_victory","state_victory",&l_result,true);
        ArenaPlayer temp = listPlayers[pos-1];
        if((temp.power > maxPower) && l_result) ;//save whitelist
        if(temp.blackList && l_result) ;//save whitelist delete blacklist
        controller->compareSample("battle/end/quick","sample_defeat","state_victory",&l_result,true);
        if(temp.whiteList && l_result) ; //save bl delete wl
        if(l_result) ; //save bl
        //
        controller->clickEsc();
        Sleep(500);
        controller->skipEvent();
    }
    else controller->clickButton("battle/arena","button_start");
}

void Arena::checkSettings(bool *result) {
    emit controller->errorLogging("Проверка настроек");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    if(settings->history_power < 1) {
        err.value = false;
        err.errorMessage = "Неправильные настройки. ИМ < 1";
        return;
    }
    switch(settings->modeTicket) {
    case 0:{
        controller->compareSample("arena/main","sample_apples","state_apples",&l_result);
        if(!l_result) controller->clickButton("arena/main","button_apples");
        break;
    }
    case 1:{
        controller->compareSample("arena/main","sample_ticket","state_ticket",&l_result);
        if(!l_result) controller->clickButton("arena/main","button_ticket");
        break;
    }
    case 2://{пока что не работает}
    case 3://туда же
    default:{
        err.value = false;
        err.errorMessage = "Неправильные настройки расходников";
        return;
    }
    }

}

void Arena::checkPower(const Mat &object, bool *result) {
    emit controller->errorLogging("Проверка мощи отряда арены");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    if(object.empty()){
        err.value = false;
        err.errorMessage = "Пустое изображение на входе в проверку мощи";
        return;
    }
    Mat temp;
    controller->changeColor(object,&temp,&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Не удалось поменять цвета картиночек...";
        return;
    }
    int number = -1;
    emit controller->Recognize(temp,number);
    if(number < 0 && number > settings->history_power) {
        err.value = false;
        return;
    }
}

void Arena::savePower(bool *result) {
    emit controller->errorLogging("Спасаю ИМ");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    int x = 0;
    while(!l_result || x < 3){
        controller->clickSwipe({460,180,0,0},{460,400,0,0},&l_result);
        controller->Screenshot();
        checkPower(controller->cutImage(),&l_result);
        if(!l_result) x++;
    }
    if(x == 3) {
        err.value = false;
        err.errorMessage = "Не получилось сохранить ИМ";
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
    emit controller->Logging(("Текущий этап: " + QString::number(currentStage)));
    for(int i = 0,n = listPlayers.size(); i < n; i++) {
        ArenaPlayer temp = listPlayers[i];
        QString str = "[" + QString::number(i) + "] Место: " + QString::number(temp.place);
        str += " Мощь: " + QString::number(temp.power) + "\n";
        str += " BL: " + QString(temp.blackList ? "+" : "-")
            +  " WL: " + QString(temp.whiteList ? "+" : "-");
        str += " Статус: ";
        switch (temp.status){
        case (State::AVAILABLE): {
            str += "Доступен для атаки";
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
        emit controller->Logging(str);
    }
}

void Arena::setUnitsSet(bool *result) {}
void Arena::savePlayerToBlackList(const Mat &player, int playerPower){}
void Arena::savePlayerToWhileList(const Mat &player, int playerPower){}
void Arena::loadLists(int playerID) {}
