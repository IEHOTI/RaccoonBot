#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"

#include <QThread>

void Controller::skipEvent() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        checkEvent(&l_result);
        if(l_result) {
            clickEsc();
            QThread::msleep(2000);
        }
    } while(l_result);
}

void Controller::refreshMainPage(ErrorList *result) {
    emit errorLogging("===Обновление главной страницы===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    checkMainPage(&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    clickButton("main","button_friends",&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int x = 0;
    do {
        compareSample("top_players","sample","compare",&l_result,true);
        if(l_result) break;
        else x++;
        QThread::msleep(500);
    } while(x < 100);
    if(x == 100) {
        observer.value.warning = m_Warning::FAIL_COMPARE;
        observer.comment = "top_players";
        return;
    }
    do{
        compareSample("top_players","sample_visit","compare_visit",&l_result,true);
        if(l_result) click();
        else {
            clickPosition(cv::Rect(480,200,0,0));
            QThread::msleep(500);
            compareSample("load","sample","compare",&l_result,true);
            //if l_result break;
        }
    } while(!l_result);
    do {
        compareSample("top_players","sample_top","compare_top",&l_result,true);
        if(!l_result) QThread::msleep(500);
    } while(!l_result);
    clickEsc();
    QThread::msleep(500);
    checkLoading();
    checkMainPage(&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
}

void Controller::fixBattleSettings() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do{
        compareSample("load","sample","compare",&l_result,true);
        if(l_result) break;
        else QThread::msleep(1000);
    } while(true);
    checkLoading();
    compareSample("battle/general","sample","compare_hero",&l_result,true);
    if(l_result) click();
    compareSample("battle/general","sample","compare_damage",&l_result);
    if(l_result) click();
    compareSample("battle/general","sample","compare_effects",&l_result);
    if(l_result) click();
    do{
    compareSample("battle/general","sample","compare_speed",&l_result,true);
        if(!l_result) {
            clickButton("battle/general","button_speed");
            QThread::msleep(500);
        }
    } while(!l_result);
}

bool Controller::fixSimpleError() {
    emit errorLogging("===SimpleFix===");
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    //step 1
    checkEvent(&l_result);
    if(l_result) {
        skipEvent();
        return true;
    }

    //step2
    compareSample("warnings/general","sample_daily","compare_daily",&l_result,true);
    if(l_result) {
        clickButton("warnings/general","button_daily",&l_result);
        if(l_result) return true;
    }

    //step3
    compareSample("warnings/general","sample_goblin","compare_goblin",&l_result,true);
    if(l_result){
        //тут проверка будет на рекламы, но пока что тупо Esc
        clickEsc();
        return true;
    }

    //step 4, ваш замок уязвим
    compareSample("warnings/general","sample_vulnerable","compare_vulnerable",&l_result,true);
    if(l_result) {
        clickButton("warnings/general","button_vulnerable",&l_result);
        if(l_result) return true;
    }

    return false;
}

void Controller::fixDifficultError() {
    emit errorLogging("===DifficultFix===");
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    //step 1
    compareSample("warnings/general","sample_sleep","compare_sleep",&l_result,true);
    if(l_result) {
        clickButton("warnings/general","button_sleep",&l_result);
        if(l_result) throw FixerException("Sleep fix",false);
    }

    //step2
    compareSample("warnings/general","sample_under_attack","compare_under_attack",&l_result,true);
    if(l_result) {
        clickButton("warnings/general","button_under_attack",&l_result);
        if(l_result) throw FixerException("Under attack fix",false);
    }

    //step3
    compareSample("warnings/general","sample_device","compare_device",&l_result,true);
    if(l_result) {
        Logging("Вход с другого устройства",true);
        Logging("Ожидание 30 минут");
        QThread::sleep(1800);///1800
        Logging("Продолжаем работу.",true);

        clickButton("warnings/general","button_device",&l_result);
        if(l_result) throw FixerException("Device fix",false);
    }

    //step 4, тех работы
    // compareSample("warnings/general","sample_","compare_daily",&l_result,true);
    // if(l_result) {
    //     clickButton("warnings/general","button_daily",&l_result);
    //     if(l_result) return true;
    // }

    return;
}

void Controller::fixIncorrectImageError() {
    emit errorLogging("===IncorrectImageFix===");
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    int x = 0;

    while(x < 15) {
        clickEsc();
        // compareSample("warnings/general","sample_exit","compare_exit",&l_result,true);
        // if(l_result) {
        //     clickButton("warnings/general","button_exit",&l_result);
        //     if(l_result) throw FixerException("Image fix",false);
        // }
        checkMainPage(&l_result);
        if(!l_result) ++x;
        QThread::msleep(1000);
    }
    return;
}

void Controller::fixErrors() {
    emit errorLogging("===Fixer===");

    if(fixSimpleError()) return;

    fixDifficultError();

    fixIncorrectImageError();

    //experimental
    getGameError();
    throw FixerException("Full fix, reload emulator",true);
}
