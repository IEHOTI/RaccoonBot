#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

void Controller::skipEvent() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        checkEvent(&l_result);
        if(l_result) {
            clickEsc();
            QThread::msleep(1000);
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

void Controller::fixPopUpError(ErrorList *result){
    emit errorLogging("===Фикс всплывающих окон===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    QList<QString> temp;
    temp << "device" << "sleep" << "daily" ;
    for(int i = 0; i < temp.size(); i++){
        compareSample("warnings/general","sample_"+temp[i],"compare_"+temp[i],&l_result,true);
        if(l_result){
            clickButton("warnings/general","button_" + temp[i],&l_result,2);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            compareSample("load","sample","compare",&l_result,true);
            if(l_result){
                checkLoading();
                return;
            }
        }
    }
    QThread::msleep(1000);
    compareSample("warnings/general","sample_goblin","compare_goblin",&l_result,true);
    if(l_result){
        //тут проверка будет на рекламы, но пока что тупо Esc
        clickEsc();
        return;
    }
}

void Controller::fixGameError(ErrorList *result) {
    if(result && *result) return;
    emit errorLogging("===Фикс полный===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    fixPopUpError(&l_result);
    if(l_result) return;
    //тут будет проверка и фикс всплывающей фигни "ваш замок уязвим" if(l_result) return;
    //тут фикс размера эмуля после перезапуска
    //если дошло до сюда тут сигнал послать чтобы он перезапустил эмуль emit endEmu emit startEmu в GUI
    //связать Emulator и this
    do{ compareSample("load","sample","compare",&l_result,true); }
    while(!l_result);
    checkLoading();
    checkPreMainPage();
    checkMainPage();
    return;
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
