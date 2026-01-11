#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "ImageLibrary/ImageLibrary.h"
#include "CustomError/Exception.h"

#include <QThread>
#include <QImage>
#include <QCoreApplication>

void Controller::isEmpty(ErrorList *result) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    if(m_main == NULL || m_game == NULL) {
        observer.value.error = m_Error::NO_ACTIVE_EMULATOR;
        return;
    }
}

void Controller::isValidSize(ErrorList *result) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    RECT temp;
    int width,height;
    GetWindowRect(m_game,&temp);
    width = temp.right - temp.left;
    height = temp.bottom - temp.top;
    if(width != 900 || height != 600) {
        observer.value.warning = m_Warning::WRONG_EMULATOR_SIZE;
        return;
    }
}

void Controller::isValidPos(ErrorList *result) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    RECT temp;
    GetWindowRect(m_game,&temp);
    if(temp.left != 1 || temp.top != 33) {
        observer.value.warning = m_Warning::WRONG_EMULATOR_POS;
        return;
    }
}

void Controller::checkLoading() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do{
        compareSample("load","sample","compare",&l_result,true);
        if(!l_result) break;
        else QThread::msleep(1000);
    } while(true);
    QThread::msleep(500);
}

void Controller::checkGameLoading() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        compareSample("load","sample_open","compare_open",&l_result,true);
        if(!l_result) break;
        else QThread::msleep(1000);
    } while(true);
    do {
        compareSample("load","sample_logo","compare_logo",&l_result,true);
        if(!l_result) break;
        else QThread::msleep(1000);
    } while(true);
    checkLoading();
}

void Controller::checkPreMainPage() {
    checkGameLoading();
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    checkEvent(&l_result);
    if(l_result) skipEvent();
    compareSample("load", "sample_mail", "compare_mail", &l_result,true);
    if (l_result) {
        clickButton("load","button_close_mail",&l_result);
        QThread::msleep(500);
    }

    //ne pomny gde pass viskakivaet + сделать выходы из циклов
    do {
        compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if(l_result) {
            clickEsc();
            QThread::msleep(3000);
        }
    } while(l_result);

    do {
        compareSample("load", "sample_offer", "compare_offer", &l_result, true);
        if(l_result) {
            clickEsc();
            QThread::msleep(3000);
        }
    } while(l_result);

    do {
        compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if(l_result) {
            clickEsc();
            QThread::msleep(3000);
        }
    } while(l_result);
}

void Controller::checkMainPage(ErrorList *result) {
    QThread::msleep(500);
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    compareSample("main","sample","compare",&l_result,true);
    if(!l_result){
        emit errorLogging("Первая проверка провалена.");
        compareSample("main","sample","compare_1",&l_result,true);
        if(!l_result){
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "fail 2x check main page";
            return;
        }
    }
}

void Controller::checkEvent(ErrorList *result) {
    LocalLogging("Check Event");
    //if(!event) return;
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    int x = 0;
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    setMask("event/compare",&l_result);
    if(!l_result) {
        observer.value = l_result;
        return;
    }

    //convertImage(QImage((mainPath + "/event/sample.png")), &m_object,&l_result);
    setMatObject("event/sample",&l_result);
    if(!l_result) {
        observer.value = l_result;
        return;
    }
    findObject();
    cv::Mat find = cutImage();
    do {
        Screenshot();
        compareObject(0.035,&find,&m_object,&l_result);
        if(l_result) break;
        else {
            x++;
            QThread::msleep(500);
        }
    } while(x < 2);
    if(x == 2) {
        observer.value.warning = m_Warning::NO_EVENT;
        observer.print = false;
        return;
    }
}

void Controller::checkSettings(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    clickButton("main","button_settings",&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int x = 0;
    do {
        compareSample("settings","sample","compare",&l_result,true);
        if(l_result) break;
        else {
            x++;
            QThread::msleep(500);
        }
    } while(x < 10);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    compareSample("settings","sample","state_fps",&l_result);
    if(!l_result) clickButton("settings","button_fps",&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    compareSample("settings","sample","state_lang",&l_result);
    if(!l_result) {
        clickButton("settings","button_lang",&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        QThread::msleep(1000);
        compareSample("settings","sample_change_lang","compare_change_lang",&l_result,true);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        clickButton("settings","button_en",&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        compareSample("settings","sample_confirm","compare_confirm",&l_result,true);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        clickButton("settings","button_yes",&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
    } else clickEsc(nullptr,2);
}

void Controller::checkMap(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    do {
        compareSample("map","sample","compare",&l_result,true);
        if(!l_result) {
            x++;
            QThread::msleep(1000);
        }
        else return;
    } while (x < 15);
    observer.value.warning = m_Warning::FAIL_CHECK;
    observer.comment = "map";
    return;
}
