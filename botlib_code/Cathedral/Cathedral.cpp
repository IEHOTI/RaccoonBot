#include "Cathedral/Cathedral.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

void Cathedral::fullGamePass(ErrorList *result) {
    emit controller->Logging("Начато полное прохождение собора");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.");
        while(true) {
            findWaypoint(&l_result);
            if (currentStage == 4) return;
            else if(l_result.warning == m_Warning::UNKNOWN) break; // stage end and not 4
            else if(!l_result) { // other warnings
                observer.value = l_result;
                observer.comment = "Проблема: не обнаружены точки или кнопки перехода";
                return;
            }
            controller->click(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            int type = -1;
            checkWaypoints(type,&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            attackWaypoints(type,&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
        }
        checkStage(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
    }
}

void Cathedral::bossGamePass(ErrorList *result) {
    emit controller->Logging("Начато быстрое прохождение собора");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.");
        controller->Screenshot();
        controller->setSample(controller->getMatObject(),&l_result);

        controller->setMatObject("dark/map/sample_0",&l_result);
        if(!l_result) NoPrintError(&observer,l_result);

        controller->setMask("dark/map/find_0");
        controller->findObject(nullptr,&l_result);
        if(!l_result) NoPrintError(&observer,l_result);

        cv::Mat tempMat = controller->cutImage();
        controller->compareObject(0.12,&tempMat,nullptr,&l_result);
        QVector<cv::Rect> bossWay = getBossWay(controller->getRect());
        int n = bossWay.count();
        emit controller->errorLogging("==Начальная точка: " + QString::number(controller->getRect().x)
                                      + ";" + QString::number(controller->getRect().y));
        if(n > 1) {
            int errorCount = 0;
            int type = -1;
            for(int i = 0; i < n; i++){
                bool battle = true;
                controller->clickPosition(bossWay[i]);
                checkWaypoints(type,&l_result);
                if(!l_result){
                    i -= 2; // ??
                    errorCount++;
                    if(errorCount == 3) n = -5;
                    continue;
                }
                else errorCount = 0;
                attackWaypoints(type,nullptr,&battle);
                if(!battle) i--;//если поражение снова в ту точку
            }
            checkEndStage();
            if (currentStage == 4) return;
        }
        else{
            emit controller->Logging("Этаж был начат. Полное прохождение текущего этажа.");
            while(true) {
                findWaypoint(&l_result);
                if (currentStage == 4) return;
                else if(l_result.warning == m_Warning::UNKNOWN) {
                    controller->LocalLogging("======Этаж кончился, переход к фаст прохождению");
                    break; // stage end and not 4
                }
                else if(!l_result) {
                    observer.value = l_result;
                    observer.comment = "Проблема: не обнаружены точки или кнопки перехода";
                    return;
                }
                emit controller->errorLogging("Обнаружил точку на координатах: " + QString::number(controller->getRect().x)
                                              + ";" + QString::number(controller->getRect().y));
                controller->click(&l_result);
                if(!l_result){
                    observer.value = l_result;
                    observer.print = false;
                    return;
                }
                int type = -1;
                checkWaypoints(type,&l_result);
                if(!l_result){
                    observer.value = l_result;
                    observer.print = false;
                    return;
                }
                attackWaypoints(type,&l_result);
            }
        }
        checkStage(&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
    }
}



