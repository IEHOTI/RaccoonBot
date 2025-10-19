#include "Lighthouse/Lighthouse.h"
#include "Lighthouse/LighthouseSettings.h"
#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"
#include "Controller/Controller.h"

#include <QThread>

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
            controller->compareSample("lighthouse","sample","compare",&l_result,true);
            if(l_result) break;
            QThread::msleep(1000);
            x++;
            if(x > 50) NoPrintError(&observer,l_result);
        }while(!l_result);
        if(!l_result) NoPrintError(&observer,l_result);
        int lose_count = 0;
        int refreshing = 0;
        //free attack
        for(int i = 0, attack_pos = 0; i < 5; i++){
            cv::Mat temp;
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
            else {
                QThread::msleep(500);
                controller->skipEvent();
            }
            refreshEnemy(i,lose_count,refreshing,&l_result);
            if(!l_result) NoPrintError(&observer,l_result);
        }
        lose_count = 0; // пока так, потом при добавлении ЧС убрать
        //diamond attack
        for(int i = 0, voidint = -1, attack_pos = 0; i < settings->diamondAttack; i++){
            cv::Mat temp;
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
            else {
                QThread::msleep(500);
                controller->skipEvent();
            }
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
