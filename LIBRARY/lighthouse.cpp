#include "lighthouse.h"

Lighthouse::Lighthouse(Controller *g_controller, QObject *parent)
    : Task{parent}
{
    controller = g_controller;
    settings = nullptr;
    localPath = g_controller->getMainPath();
    stop_flag = false;
    myPower = 0;
}

Lighthouse::~Lighthouse() {
}

void Lighthouse::Pause() {
    throw PauseException();
}

void Lighthouse::Stop() {
    throw StopException();
}

void Lighthouse::Initialize(TaskSettings *setting, ErrorList *result) {
    emit controller->Logging("Инициализация настроек маяка.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    if (!setting) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "nullptr settings";
        return;
    }
    if(auto* lighthouse = dynamic_cast<LighthouseSettings*>(setting)) settings = lighthouse;
    else {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "different type lighthouse and unknown";
        emit controller->Logging("Произошла ошибка. Отправьте отчёт об ошибке.");
        settings = nullptr;
    }
    return;
}

void Lighthouse::Start(ErrorList *result) {
    emit controller->Logging("Задание [Маяк] начато.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    try {
        setUnitsSet(&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
        controller->checkMainPage(&l_result);
        if(!l_result){
            controller->fixGameError(&l_result);
            if(!l_result) NoPrintError(&observer,l_result);
        }
        controller->clickButton("main","button_map");
        controller->checkMap(&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
        controller->clickMapButton("sample_right","button_lighthouse",&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
        stop_flag = false;
        int x = 0;
        do{
            controller->compareSample("lighthouse","sample","compare",&l_result);
            if(l_result) break;
            Sleep(1000);
            x++;
            if(x > 50) NoPrintError(&observer,l_result);
        }while(!l_result);
        if(!l_result) NoPrintError(&observer,l_result);
        int lose_count = 0;
        int refreshing = 0;
        for(int i = 0, attack_pos = 0; i < 5; i++){
            Mat temp;
            scanEnemy(attack_pos,temp,&l_result);
            if(l_result.warning == m_Warning::MORE_THAN_HISTORY_POWER) {
                refreshEnemy(i,lose_count,refreshing,&l_result);
                if(!l_result) NoPrintError(&observer,l_result);
                continue;
            }
            else if(!l_result) NoPrintError(&observer,l_result);
            attackEnemy(attack_pos, i, &l_result);
            if(i == INT_MAX) break;
            else if(!l_result) NoPrintError(&observer,l_result);
            bool battle = false;
            checkBattleResult(&battle);
            if(!battle) lose_count++;//add temp to blacklist
            Sleep(500);
            controller->skipEvent();
            refreshEnemy(i,lose_count,refreshing,&l_result);
            if(!l_result) NoPrintError(&observer,l_result);
        }
        lose_count = 0; // пока так, потом при добавлении ЧС убрать
        for(int i = 0, voidint = -1, attack_pos = 0; i < settings->diamondAttack; i++){
            Mat temp;
            scanEnemy(attack_pos,temp,&l_result);
            if(l_result.warning == m_Warning::MORE_THAN_HISTORY_POWER) {
                refreshEnemy(i,lose_count,refreshing,&l_result);
                if(!l_result) NoPrintError(&observer,l_result);
                continue;
            }
            else if(!l_result) NoPrintError(&observer,l_result);
            attackEnemy(attack_pos,voidint,&l_result);
            if(!l_result) NoPrintError(&observer,l_result);
            bool battle = false;
            checkBattleResult(&battle);
            if(!battle) lose_count++;//add temp to blacklist
            Sleep(500);
            controller->skipEvent();
            refreshEnemy(i,lose_count,refreshing,&l_result);
            if(!l_result) NoPrintError(&observer,l_result);
        }
        controller->clickEsc();
        controller->clickButton("map","button_close");
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

void Lighthouse::checkBattleResult(bool *battle) {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
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

void Lighthouse::scanEnemy(int &attack_pos, Mat &resultScan, ErrorList *result){
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    for(int i = 1; i < 6; i++) {
        controller->compareSample("lighthouse","sample","button_" + QString::number(i),&l_result);
        if(l_result){
            controller->setMask("lighthouse/power_" + QString::number(i));
            controller->findObject();
            int power = 0;
            controller->Recognize(controller->cutImage(),power);
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

void Lighthouse::attackEnemy(int pos,int &count, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->clickButton("lighthouse","button_" + QString::number(pos));
    int x = 0;
    do {
        controller->compareSample("battle/lighthouse","sample","compare",&l_result);
        if(!l_result){
            x++;
            if(x == 50) NoPrintError(&observer,l_result);
            Sleep(1000);
        }
    } while(!l_result);
    if(settings->premiumStatus) {
        controller->clickButton("battle/lighthouse","button_qstart");
        checkWarning();
        controller->compareSample("lighthouse","no_energy","compare_energy",&l_result);
        if(l_result) {
            if(count != -1) {
                count = INT_MAX;
                controller->clickEsc();
                controller->clickEsc();
                return;
            }
            controller->clickButton("lighthouse","button_energy");
        }
    }
    else {}

}

void Lighthouse::refreshEnemy(int &count_attack,int count_lose, int &count_refresh, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->compareSample("lighthouse","sample_refresh","compare_refresh",&l_result);
    if(l_result) {
        switch (settings->modeRefresh) {
        case -1: {
            if(count_lose == 1) controller->clickButton("lighthouse","button_refresh");
            break;
        }
        case 3:{
            if(count_attack == 2) controller->clickButton("lighthouse","button_refresh");
            break;
        }
        case 5:{
            if(count_attack == 4) controller->clickButton("lighthouse","button_refresh");
            break;
        }
        default:{
            observer.value = l_result;
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "wrong modeRefresh setting";
            return;
        }
        }
        if(!(observer.value)) controller->clickButton("lighthouse","button_refresh");
        count_refresh++;
    }
    else {
        if(count_refresh > settings->diamondRefresh) {
            if(!(observer.value)) count_attack = INT_MAX;
            return;
        }
        switch (settings->modeRefresh) {
        case -1: {
            if(count_lose == 1) controller->clickButton("lighthouse","button_refresh");
            break;
        }
        case 3:{
            if(count_attack == 2) controller->clickButton("lighthouse","button_refresh");
            break;
        }
        case 5:{
            if(count_attack == 4) controller->clickButton("lighthouse","button_refresh");
            break;
        }
        default:{
            observer.value = l_result;
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "wrong modeRefresh setting";
            return;
        }
        }
        if(!(observer.value)) controller->clickButton("lighthouse","button_refresh");
        count_refresh++;
    }
    observer.value = l_result;
}

void Lighthouse::setUnitsSet(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->checkMainPage(&l_result);
    if(l_result.error == m_Error::FAIL_INIT) {
        controller->refreshMainPage(&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
    }
    controller->findBarracks(&l_result);
    if(!l_result) NoPrintError(&observer,l_result);
    for(int i = 0; i < settings->squadSet.size();i++){
        controller->setUnitSet(i,settings->squadSet[i],&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
    }
    controller->findBarracks(&l_result);
    if(!l_result) {
        controller->refreshMainPage(&l_result);
        if(!l_result) NoPrintError(&observer,l_result);
    }
    else controller->entryBarracks(&l_result);

    if(!l_result) NoPrintError(&observer,l_result);

    controller->setMask("squad/main/power");
    controller->findObject();
    checkPower(controller->cutImage(),&l_result);
    if(l_result.warning == m_Warning::MORE_THAN_HISTORY_POWER) saveHistoryPower(&l_result); // доработать
    else if(!l_result) NoPrintError(&observer,l_result);
}

void Lighthouse::checkPower(const Mat &object, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if(object.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "empty object";
        return;
    }
    Mat temp;
    controller->changeColor(object,&temp,&l_result);
    if(!l_result) NoPrintError(&observer,l_result);
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
    settings->user_power = number;
}

void Lighthouse::checkWarning() {
    Sleep(500);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->compareSample("warnings/lighthouse","sample","compare",&l_result);
    if(l_result) controller->clickButton("warnings/lighthouse","button_yes");
    Sleep(500);
}

void Lighthouse::saveHistoryPower(ErrorList *result) {}

void Lighthouse::savePlayerToBlackList(const Mat &player, int playerPower) {}
void Lighthouse::savePlayerToWhileList(const Mat &player, int playerPower) {}
void Lighthouse::loadLists(int playerID) {}
