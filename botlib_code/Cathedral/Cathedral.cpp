#include "Cathedral/Cathedral.h"
#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "qthread.h"

void Cathedral::fullGamePass(ErrorList *result) {
    emit controller->Logging("Начато полное прохождение собора");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.", false);
        while(true) {
            findWaypoint(&l_result);
            if (currentStage == 4) return;
            else if(l_result.warning == m_Warning::UNKNOWN) break; // stage end and not 4
            else if(!l_result) controller->fixErrors();

            int type = -1;
            do {
                controller->click(&l_result);
                if(!l_result) NoPrintError(&observer,l_result);

                checkWaypoints(type,&l_result);
                if(!l_result) continue;

                attackWaypoints(type,&l_result);
                if(!l_result) NoPrintError(&observer,l_result);
            } while(type < 0);
        }
        checkStage(&l_result);
    }
}

void Cathedral::bossGamePass(ErrorList *result) {
    emit controller->Logging("Начато быстрое прохождение собора");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.",false);
        controller->Screenshot();
        controller->setSample(controller->getMatObject(),&l_result);
        if(!l_result) NoPrintError(&observer,l_result);

        controller->setMatObject("dark/map/sample_0",&l_result);
        if(!l_result) NoPrintError(&observer,l_result);

        controller->setMask("dark/map/find_0");
        if(!l_result) NoPrintError(&observer,l_result);

        controller->findObject(nullptr,&l_result);
        if(!l_result) NoPrintError(&observer,l_result);

        cv::Mat tempMat = controller->cutImage();
        controller->compareObject(0.12,&tempMat,nullptr,&l_result);

        QVector<cv::Rect> bossWay = getBossWay(controller->getRect());
        emit controller->errorLogging("=====Начальная точка: " + QString::number(controller->getRect().x)
                                      + ";" + QString::number(controller->getRect().y));
        int n = bossWay.count();
        if(n > 1) {
            int errorCount = 0;
            int type = -1;
            for(int i = 0; i < n; ++i){
                bool battle = true;
                do {
                    controller->clickPosition(bossWay[i]);
                    checkWaypoints(type,&l_result);
                    if(!l_result){
                        if(++errorCount == 5) controller->fixErrors();
                        else QThread::msleep(500);
                    } else {
                        errorCount = 0;
                        break;
                    }
                } while(!l_result);
                attackWaypoints(type,nullptr,&battle);
                if(!battle) i--;//если поражение снова в ту точку
            }
            checkEndStage();
            if (currentStage == 4) return;
        }
        else{
            emit controller->Logging("Этаж был начат. Полное прохождение текущего этажа.",false);
            while(true) {
                bool flag = false;
                do {
                    findWaypoint(&l_result);
                    if (currentStage == 4) return;
                    else if(l_result.warning == m_Warning::UNKNOWN) {
                        controller->LocalLogging("======Этаж кончился, переход к фаст прохождению");
                        flag = true;
                        break; // stage end and not 4
                    }
                    else if(!l_result) controller->fixErrors();
                }while(!l_result);
                if(flag) break;

                emit controller->errorLogging("Обнаружил точку на координатах: " + QString::number(controller->getRect().x)
                                              + ";" + QString::number(controller->getRect().y));
                int type = -1;
                do {
                    controller->click(&l_result);
                    if(!l_result) NoPrintError(&observer,l_result);

                    checkWaypoints(type,&l_result);
                    if(!l_result) continue;

                    attackWaypoints(type,&l_result);
                    if(!l_result) NoPrintError(&observer,l_result);
                } while(type < 0);
            }
        }
        checkStage(&l_result);
    }
}



