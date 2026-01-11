#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "Task/TaskSettings.h"

#include <QThread>

void Controller::openHeroPage(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    clickButton("main","button_user",&l_result,3);
    if(!l_result) NoPrintError(&observer,l_result);
    int x = 0;
    do {
        compareSample("user","sample","compare",&l_result,true);
        if(!l_result) {
            x++;
            if(x == 10) {
                observer.value.warning = m_Warning::FAIL_PAGE;
                observer.comment = "openHeroPage->No user Page";
                return;
            }
            else QThread::msleep(1000);
        }
    } while(!l_result);
    clickButton("user","button_battles");
    compareSample("user","sample_battles","compare_battles",&l_result,true);
    if(!l_result) NoPrintError(&observer,l_result);

    clickButton("user","button_hero");
    QThread::msleep(500);
    compareSample("hero","sample","compare",&l_result,true);
    if(!l_result) NoPrintError(&observer,l_result);
}

void Controller::setHeroSet(typeSet set, ErrorList *result){ // errorlist?
    if(set == typeSet::NOT_TOUCH) return;
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    openHeroPage(&l_result);
    if(!l_result) NoPrintError(&observer,l_result);

    openAnySets(&l_result);
    if(!l_result) {
        observer.value.warning = m_Warning::FAIL_PAGE;
        observer.comment = "setHeroSet->openAnySets";
        return;
    }

    QString pagePath = "squad/main/unit";
    switch (set) {
    case typeSet::UNEQUIP : {
        compareSample(pagePath,"sample_set_3","state_unequip",&l_result);
        if(l_result) click();
        break;
    }
    case typeSet::SET_1 : {
        compareSample(pagePath,"sample_set_0","state_set",&l_result);
        if(l_result) {
            Logging("У героя отсутствуют наборы вещей.",false);
            break;
        }
        clickButton("hero","button_set_1");
        break;
    }
    case typeSet::SET_2 : {
        compareSample(pagePath,"sample_set_0","state_set",&l_result);
        if(l_result) {
            Logging("У героя отсутствует первый набор вещей.",false);
            break;
        }
        compareSample(pagePath,"sample_set_1","state_set",&l_result);
        if(l_result) {
            Logging("У героя отсутствует второй набор вещей.",false);
            break;
        }
        clickButton("hero","button_set_2");
        break;
    }
    case typeSet::SET_3 : {
        compareSample(pagePath,"sample_set_3","state_set",&l_result);
        if(!l_result) {
            Logging("У героя отсутствует третий набор вещей.",false);
            break;
        }
        clickButton("hero","button_set_3");
        break;
    }
    default: {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "hero set out of range";
        return;
    }
    }

    compareSample("hero","sample_confirm_set","compare_confirm_set",&l_result,true);
    if(!l_result) NoPrintError(&observer,l_result);

    clickButton("hero","button_confirm_set");
    clickEsc();
    //end?
}
void Controller::setHeroRelics(ErrorList *result, const QList<QString> *relicsList) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    if(relicsList->size() < 0 || relicsList->size() > 5) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "setHeroRelics->oversize list";
        return;
    }

    openHeroPage(&l_result);
    if(!l_result) NoPrintError(&observer,l_result);

    openAnySets(&l_result);
    if(!l_result) {
        observer.value.warning = m_Warning::FAIL_PAGE;
        observer.comment = "setHeroSet->openAnySets";
        return;
    }

    compareSample("squad/main/unit","sample_set_3","state_unequip",&l_result,true);
    if(!l_result){
        observer.value.warning = m_Warning::FAIL_PAGE;
        observer.comment = "setHeroRelics->not unequip";
        return;
    }
    click();
    clickEsc();

    compareSample("hero","sample_empty","compare_empty",&l_result,true);
    if(!l_result) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "no empty relic";
        return;
    }

    QList<QString> temp = relicsList->toList();

    for(int count = 1; count < 9; count++) {
        Screenshot();
        setSample(getMatObject());
        for(int i = 0, n = temp.size(); i < n;i++) {
            QString tempString = temp[i];
            setMatObject("hero/" + tempString);
            compareObject(0.02,nullptr,nullptr,&l_result);
            if(l_result){
                //нашел
                click();
                QThread::msleep(500);
                compareSample("hero","sample_equip_relic","compare_equip_relic",&l_result,true);
                if(!l_result) {
                    observer.value.error = m_Error::FAIL_INIT;
                    observer.comment = "fail set relic: " + tempString;
                    return;
                }
                clickButton("hero","button_equip_relic");
                temp.removeOne(tempString);
                i--;
                n--;
            }
        }
        clickSwipe({680,545,0,0},{680,330,0,0});
    }
    clickEsc();
}
