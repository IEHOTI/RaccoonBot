#include "Controller/Controller.h"
#include "User/UserProfile.h"
#include "CustomError/ErrorObserver.h"

#include <QCoreApplication>
#include <QThread>

void Controller::userInitialize(userProfile *user, ErrorList *result) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        clickButton("main","button_user",&l_result);
        if(!l_result) fixErrors();
    }while (!l_result);

    int x = 0;
    do {
        compareSample("user","sample","compare",&l_result,true);
        if(!l_result) {
            ++x;
            QThread::msleep(1000);
        }
        else if(x == 10) {
            fixErrors();
            x = 0;
        }
    } while(x < 10);
    setMask("user/user_id",&l_result);
    if(!l_result) NoPrintError(&observer,l_result);

    do {
        findObject(nullptr,&l_result);
        if(!l_result) fixErrors();
    }while (!l_result);

    emit Recognize(cutImage(),user->user_ID);
    if(user->user_ID <= 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "scan user_id";
        return;
    }
    //////
    user->subscribe = typeSub::admin;

    //////
    setMask("user/user_power",&l_result);
    if(!l_result) NoPrintError(&observer,l_result);

    do{
        findObject(nullptr,&l_result);
        if(!l_result) fixErrors();
    }while (!l_result);

    emit Recognize(cutImage(),user->history_power);
    if(user->history_power <= 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "scan user_power";
        return;
    }
    compareSample("user","sample","state_prem",&l_result);
    if(l_result) user->state_premium = false;
    else user->state_premium = true;

    compareSample("user","sample","state_ads",&l_result);
    if(l_result) user->state_ads = false;
    else user->state_ads = true;
    ///
    user->leftover_time = "9999999 days for Raccoons";
    ////
    clickButton("user","button_close");
}
