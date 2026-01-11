#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "User/UserProfile.h"

#include <QThread>

void Controller::findBarracks(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    compareSample("squad/main","sample","compare",&l_result,true);
    if(!l_result){
        observer.value = l_result;
        observer.comment = "not found barrack";
        return;
    }
}

void Controller::entryBarracks(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    click();
    QThread::msleep(500);
    int x = 0;
    do{
        compareSample("squad/main","sample_info","compare_info",&l_result,true);
        if(!l_result){
            compareSample("squad/main","sample_info","compare_pals",&l_result);
            if(!l_result){
                x++;
                if(x == 10) {
                    observer.value.error = m_Error::FAIL_INIT;
                    observer.comment = "2 stage of compare info";
                    return;
                }
                QThread::msleep(1000);
            }
            else clickButton("squad/main","button_info");
        } else click();
    } while(!l_result);
    x = 0;
    do {
        compareSample("squad/main","sample_barrack","compare_barrack",&l_result,true);
        if(!l_result){
            x++;
            if(x == 50) {
                observer.value.warning = m_Warning::FAIL_COMPARE;
                observer.comment = "barrack";
                return;
            }
            else QThread::msleep(500);
        }
    } while (!l_result);
}

void Controller::scanSquadCount(userProfile *user, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    setMask("squad/main/count");
    findObject();
    cv::Mat temp;
    changeColor(cutImage(),&temp,&l_result/*,true*/);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int number = 0;
    emit Recognize(temp,number);
    if(number < 2) {
        observer.value.warning = m_Warning::FAIL_RECOGNIZE;
        observer.comment = "unit count";
        return;
    }
    user->count_units = number;
    clickEsc();
    clickEsc();
}

void Controller::setUnitSet(int index, typeSet set, ErrorList *result) {
    if(set == typeSet::NOT_TOUCH) return;
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if((index < 0) || (index > 8)) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "unit index out of range";
        return;
    }

    QString pagePath = "squad/main/unit";
    QVector<int> unitPos = {400,490,330,560,280,610,240,650};
    int yPos = 320;

    int x = 0;
    do{
        compareSample(pagePath,"sample","compare",&l_result,true);
        if(!l_result) {
            clickPosition(cv::Rect(unitPos[index],yPos,0,0),&l_result,1);
            if(!l_result){
                x++;
                if(x == 50) {
                    observer.value.warning = m_Warning::FAIL_COMPARE;
                    observer.comment = "unit sample";
                    return;
                }
                QThread::msleep(500);
            }
        }
    } while(!l_result);

    openAnySets(&l_result);
    if(!l_result) NoPrintError(&observer,l_result);

    // do{
    //     compareSample(pagePath,"sample_set","compare_set",&l_result,true);
    //     if(!l_result) {
    //         clickButton(pagePath,"button_set",nullptr,1);
    //         x++;
    //         if(x == 10){
    //             observer.value.warning = m_Warning::FAIL_COMPARE;
    //             observer.comment = "unit set sample";
    //             return;
    //         }
    //         QThread::msleep(1000);
    //     }
    // } while(!l_result);
    x = 0;
    switch (set) {
    case typeSet::UNEQUIP : {
        compareSample(pagePath,"sample_set_3","state_unequip",&l_result);
        if(l_result) click();
        break;
    }
    case typeSet::SET_1 : {
        compareSample(pagePath,"sample_set_0","state_set",&l_result);
        if(l_result) {
            Logging("У " + QString::number(index + 1) + " бойца отсутсвуют наборы вещей.",false);
            break;
        }
        clickButton(pagePath,"button_set_1");
        break;
    }
    case typeSet::SET_2 : {
        compareSample(pagePath,"sample_set_0","state_set",&l_result);
        if(l_result) {
            Logging("У " + QString::number(index + 1) + " бойца отсутсвует первый набор вещей.",false);
            break;
        }
        compareSample(pagePath,"sample_set_1","state_set",&l_result);
        if(l_result) {
            Logging("У " + QString::number(index + 1) + " бойца отсутсвует второй набор вещей.",false);
            break;
        }
        clickButton(pagePath,"button_set_2");
        break;
    }
    case typeSet::SET_3 : {
        compareSample(pagePath,"sample_set_3","state_set",&l_result);
        if(!l_result) {
            Logging("У " + QString::number(index + 1) + " бойца отсутсвует третий набор вещей.",false);
            break;
        }
        clickButton(pagePath,"button_set_3");
        break;
    }
    default: {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "unit set out of range";
        return;
    }
    }
    while(true){
        compareSample(pagePath,"sample_set","compare_set",&l_result,true);
        if(l_result) {
            x++;
            if(x == 5) {
                clickEsc();
                break;
            }
            QThread::msleep(1000);
        }
        else{
            x = 0;
            do{
                compareSample(pagePath,"sample_confirm","compare_confirm",&l_result,true);
                if(!l_result) {
                    x++;
                    if(x == 15){
                        observer.value.warning = m_Warning::FAIL_COMPARE;
                        observer.comment = "unit set confirm";
                        return;
                    }
                }
                QThread::msleep(1000);
            } while(!l_result);
            clickButton(pagePath,"button_confirm");
            x = 0;
            do{
                compareSample(pagePath,"confirm_warning","compare_confirm",&l_result,true);
                if(l_result) {
                    clickButton(pagePath,"button_yes");
                    x = 10;
                } else{
                    x++;
                    QThread::msleep(500);
                }
            } while(x < 10);
            break;
        }
    }
    x = 0;
    do{
        compareSample(pagePath,"sample","compare",&l_result,true);
        if(!l_result) {
            x++;
            if(x == 50) {
                observer.value.warning = m_Warning::FAIL_COMPARE;
                observer.comment = "unit sample after equip";
                return;
            }
            QThread::msleep(500);
        }
    } while(!l_result);
    clickEsc();
}
