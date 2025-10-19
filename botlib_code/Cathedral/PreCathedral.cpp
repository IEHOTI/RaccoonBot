#include "Cathedral/Cathedral.h"
#include "Cathedral/CathedralSettings.h"
#include "CustomError/ErrorObserver.h"
#include "Controller/Controller.h"

#include <QThread>

void Cathedral::confirmSquad(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        controller->compareSample("squad/dark","sample","compare",&l_result,true);
        if(!l_result) QThread::msleep(500);
    } while(!l_result);
    switch (settings->modeSquad) {
    case 0: {
        controller->clickButton("squad/dark","button_best",&l_result,2);
        break;
    }
    case 1: {
        controller->clickButton("squad/dark","button_previous",&l_result,2);
        break;
    }
    case 2: {
        break;
    }
    default:{
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "squad mode";
        return;
    }
    }
    controller->clickButton("squad/dark","button_start",&l_result,2);
    checkWarnings();
    int x = 0;
    do{
        controller->compareSample("load","sample","compare",&l_result,true);
        if(l_result) break;
        else{
            x++;
            if(x == 100){
                observer.value.error = m_Error::FAIL_INIT;
                observer.comment = "sample after warning";
                return;
            }
        }
        QThread::msleep(1000);
    }
    while(!l_result);
}

void Cathedral::checkMain(ErrorList *result){
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x<15){
        controller->compareSample("load","sample","compare",&l_result,true);
        if(l_result){
            checkStage(&l_result);
            if(currentStage == 0){
                observer.value = l_result;
                observer.print = false;
            }
            return;
        }
        else {
            controller->compareSample("dark","sample","compare",&l_result,true);
            if(l_result) return;
            else {
                QThread::msleep(1000);
                x++;
            }
        }
    }
    observer.value.warning = m_Warning::FAIL_PAGE;
    observer.comment = "cathedral main";
    return;
}

void Cathedral::checkSettings(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    if(settings->history_power < 1) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "history_power < 1";
        return;
    }
    switch(settings->modeDifficult) {
    case 0:{
        controller->compareSample("dark","sample_insane","state_normal",&l_result,true);
        if(!l_result) {
            controller->click(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
        }
        break;
    }
    case 1:{
        controller->compareSample("dark","sample_insane","state_hard",&l_result,true);
        if(!l_result) {
            controller->click(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
        }
        break;
    }
    case 2:{
        controller->compareSample("dark","sample_hard","state_insane",&l_result,true);
        if(!l_result) {
            controller->click(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
        }
        break;
    }
    default:{
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "settings->difficult";
        return;
    }
    }
    switch(settings->modeKey) {
    case 0:{
        controller->clickButton("dark","button_apples");
        break;
    }
    case 1:{
        controller->clickButton("dark","button_keys");
        break;
    }
    case 2://{пока что не работает}
    case 3://туда же
    default:{
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "settings->key";
        return;
    }
    }
}

QVector<cv::Rect> Cathedral::getBossWay(cv::Rect &rect){
    int code = rect.x * 1000 + rect.y;
    switch (code) {
    case 653062: {
        return {
            {653,  63, 1, 1},
            {673, 236, 1, 1},
            {618, 357, 1, 1},
            {547, 334, 1, 1},
            {594, 249, 1, 1}, // босс справа
            {535, 421, 1, 1},
            {428, 423, 1, 1},
            {328, 320, 1, 1}  // босс слева
        };
    }
    case 144314: {
        return {
            {144, 314, 1, 1},
            {218, 245, 1, 1},
            {320, 162, 1, 1},
            {402, 222, 1, 1},
            {328, 320, 1, 1}, // босс слева
            {404, 125, 1, 1},
            {513, 135, 1, 1},
            {594, 249, 1, 1}  // босс справа
        };
    }
    case 700237:{
        return {
            {700, 237, 1, 1},
            {810, 275, 1, 1}, // правый босс
            {701, 375, 1, 1},
            {615, 375, 1, 1},
            {315, 375, 1, 1},
            {208, 375, 1, 1},
            {205, 270, 1, 1},
            {110, 275, 1, 1}  // левый босс
        };
    }
    case 201237:{
        return {
            {201, 237, 1, 1},
            {110, 275, 1, 1}, // левый босс
            {208, 375, 1, 1},
            {315, 375, 1, 1},
            {615, 375, 1, 1},
            {701, 375, 1, 1},
            {713, 275, 1, 1},
            {810, 275, 1, 1}  // правый босс
        };
    }
    case 133303:{
        return {
            {133, 303, 1, 1},
            {261, 291, 1, 1},
            {332, 415, 1, 1},
            {468, 469, 1, 1}, // босс снизу
            {322, 182, 1, 1},
            {521, 145, 1, 1},
            {444,  95, 1, 1}, // босс сверху
            {375, 250, 1, 1},
            {471, 364, 1, 1},
            {551, 260, 1, 1}  // босс центр
        };
    }
    case 720106:{
        return {
            {720, 106, 1, 1},
            {630, 207, 1, 1},
            {521, 145, 1, 1},
            {444,  95, 1, 1}, // босс сверху
            {322, 182, 1, 1},
            {375, 255, 1, 1},
            {471, 364, 1, 1},
            {551, 260, 1, 1}, // босс центр
            {261, 291, 1, 1},
            {332, 415, 1, 1},
            {468, 469, 1, 1}  // босс снизу
        };
    }
    case 119241:{
        return {
            {119, 241, 1, 1},
            {269, 328, 1, 1},
            {386, 366, 1, 1},
            {463, 384, 1, 1},
            {466, 471, 1, 1}, // босс снизу
            {553, 363, 1, 1},
            {659, 323, 1, 1},
            {788, 279, 1, 1}, // босс справа
            {546, 203, 1, 1},
            {465, 167, 1, 1},
            {470,  80, 1, 1}  // босс сверху
        };
    }
    case 778238:{
        return {
            {778, 238, 1, 1},
            {659, 323, 1, 1},
            {553, 363, 1, 1},
            {463, 384, 1, 1},
            {466, 471, 1, 1}, // босс снизу
            {386, 366, 1, 1},
            {269, 328, 1, 1},
            {131, 283, 1, 1}, // босс слева
            {546, 203, 1, 1},
            {465, 167, 1, 1},
            {470,  80, 1, 1}  // босс сверху
        };
    }
    case 113231:{
        return {
            {113, 231, 1, 1},
            {245, 280, 1, 1},
            {278, 130, 1, 1},
            {470, 176, 1, 1},
            {471,  87, 1, 1}, // босс сверху
            {372, 273, 1, 1}, // босс слева
            {568, 270, 1, 1}, // босс справа
            {470, 377, 1, 1},
            {472, 470, 1, 1}  // босс снизу
        };
    }
    case 784223:{
        return {
            {784, 223, 1, 1},
            {680, 265, 1, 1},
            {645, 120, 1, 1},
            {470, 176, 1, 1},
            {471,  87, 1, 1}, // босс сверху
            {372, 273, 1, 1}, // босс слева
            {568, 270, 1, 1}, // босс справа
            {470, 377, 1, 1},
            {472, 470, 1, 1}  // босс снизу
        };
    }
    default:{
        return {{0,0,0,0}};
    }
    }
}
