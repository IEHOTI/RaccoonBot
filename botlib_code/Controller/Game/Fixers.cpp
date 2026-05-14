#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"

#include <QDateTime>
#include <QDir>

#include <QThread>

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

    //step 5, для собора, так как выбор благословения не сбрасывается Esc
    // compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
    // if(l_result) {
    //     clickButton("dark/waypoints/blessing","button_" + QString::number(1 + rand() % 3));
    //     clickButton("dark/waypoints/blessing","button_confirm");
    //     QThread::msleep(500);
    //     return true;
    // }

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
        QThread::msleep(250);
        checkLoading();
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
        QThread::msleep(500);
        compareSample("warnings/general","sample_exit","compare_exit",&l_result,true);
        checkLoading();
        if(l_result) {
            clickButton("warnings/general","button_cancel",&l_result);
            if(l_result) throw FixerException("Image fix",false);
        }
        compareSample("main","sample","compare",&l_result,true);
        if(!l_result) ++x;
        QThread::msleep(1000);
    }
    return;
}

void Controller::fixErrors() {
    emit errorLogging("===Fixer===");

    int x = 0;
    do {
    if(fixSimpleError()) return;

    fixDifficultError();

    fixIncorrectImageError();
    } while (++x < 4);

    //experimental
    //getGameError();
    Screenshot();
    const QString errDir = "C:/Utilities/Coding/HCErrors";
    QDir().mkpath(errDir);
    saveImage(errDir + "/img_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss") + ".png", m_images.getMatObject());
    throw FixerException("Full fix, reload emulator",true);
}
