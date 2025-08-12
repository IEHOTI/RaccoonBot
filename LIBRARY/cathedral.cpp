#include "cathedral.h"

Cathedral::Cathedral(Controller *g_controller, QObject* parent)  : Task(parent) {
    settings = nullptr;
    controller = g_controller;
    localPath = g_controller->getMainPath();
    nameWaypoints << "altar" << "camp" << "game" << "reflection";
    currentStage = 0;
    stop_flag = false;
}
Cathedral::~Cathedral() {}

void Cathedral::Initialize(TaskSettings *setting, ErrorList *result) {
    emit controller->Logging("Инициализация настроек собора.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    if (!setting) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "settings nullptr";
        return;
    }
    if(auto* cathedral = dynamic_cast<CathedralSettings*>(setting)) settings = cathedral;
    else {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "wrong type: cathedral -> unknown";
        emit controller->Logging("Произошла ошибка. Отправьте отчёт об ошибке.");
        settings = nullptr;
    }
    return;
}

void Cathedral::Stop() {
    stop_flag = true;
    controller->errorLogging("Остановка задания [Собор].");
}

void Cathedral::Start(ErrorList *result) {
    emit controller->Logging("Задание [Собор] начато.");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->checkMainPage(&l_result);
    if(!l_result){
        controller->fixGameError(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
    }
    controller->clickButton("main","button_map");
    controller->checkMap(&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    controller->clickMapButton("sample","button_dark",&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int k = 0;
    stop_flag = false;
    while(!stop_flag) {
        currentStage = 0;
        checkMain(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
        }
        if(currentStage == 0) {
            checkSettings(&l_result);
            if(l_result) {
                controller->clickButton("dark","button_start");
                confirmSquad(&l_result);
                if(!l_result){
                    observer.value = l_result;
                    observer.print = false;
                    return;
                }
            }
            else {
                observer.value = l_result;
                observer.print = false;
                emit controller->Logging("Задание прервано. Ошибка в настройках");
                return;
            }
            checkStage(&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
        }
        if(settings->fullGamePass) fullGamePass(&l_result);
        else bossGamePass(&l_result);
        do controller->compareSample("dark","sample_end","compare_end",&l_result,true);
        while(!l_result);
        do {
            controller->compareSample("dark","sample_end","compare_end",&l_result,true);
            if(l_result) {
                controller->clickButton("dark","button_end");
                Sleep(200);
            }
        } while(l_result);
        controller->checkLoading();
        Sleep(200);
        controller->checkEvent(&l_result);
        if(l_result) controller->skipEvent();
        k++;
        if(k == settings->count) stop_flag = true;
    }
    controller->clickButton("dark","button_close");
    controller->checkMainPage(&l_result);
    if(!l_result){
        controller->checkMap(&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        controller->clickButton("map","button_close");
    }
}

void Cathedral::checkPower(const Mat &object, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    Mat l_object;
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->changeColor(object,&l_object,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int power = 0;
    emit controller->Recognize(l_object,power);
    if(power <= 0) {
        observer.value.warning = m_Warning::FAIL_RECOGNIZE;
        controller->clickButton("battle/dark","button_best",&l_result);
        return;
    }
    if(power > settings->history_power) {
        controller->clickSwipe({468,194,0,0},{467,394,0,0},&l_result);
        if(!l_result) {
            observer.value.warning = m_Warning::MORE_THAN_HISTORY_POWER;
            observer.comment = QString::number(power) + " vs " + QString::number(settings->history_power);
            return;
        }
    }
    return;
}

void Cathedral::confirmSquad(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do {
        controller->compareSample("squad/dark","sample","compare",&l_result,true);
        if(!l_result) Sleep(500);
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
    Sleep(1000);
    controller->compareSample("load","sample","compare",&l_result,true);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        emit controller->Logging("Не удалось начать собор. Отправьте сообщение об ошибке.");
        return;
    }
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
                Sleep(1000);
                x++;
            }
        }
    }
    observer.value.warning = m_Warning::FAIL_PAGE;
    observer.comment = "cathedral main";
    return;
}

void Cathedral::checkStage(ErrorList *result) {
    if (currentStage <= 0 && currentStage >= 4) return;
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x < 3) {
        controller->checkLoading();

        controller->compareSample("dark/waypoints","sample_complete","compare_complete",&l_result,true,0.01);
        if(l_result) controller->clickButton("dark/waypoints","button_close");

        controller->compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
        if(l_result) {
            controller->clickButton("dark/waypoints/blessing","button" + QString::number(1 + rand() % 3));
            controller->clickButton("dark/waypoints/blessing","button_confirm");
        }

        controller->Screenshot();
        for(int i = 0; i < 3; i++) {
            controller->compareSample("dark/waypoints","stage_" + QString::number(i+1),
                                      "compare_stage",&l_result,false,0.03);
            if(l_result) {
                currentStage = i+1;
                return;
            }
        }
        x++;
    }
    observer.value = l_result;
    observer.comment = "Cathderal floor";
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

void Cathedral::checkWarnings() {
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x < 6) {
        controller->compareSample("warnings/dark","sample_1","compare",&l_result,true,0.01);
        if(!l_result){
            controller->compareSample("warnings/dark","sample_2","compare",&l_result,false,0.01);
            if(!l_result) {
                x++;
                Sleep(200);
                continue;
            }
        }
        controller->clickButton("warnings/dark","button_yes");
        return;
    }
}

void Cathedral::findWaypoint(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while(x<5) {
        Sleep(200);
        controller->Screenshot();
        controller->setSample(controller->getMatObject(),&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        for(int i = 0; i < 7; i++) {
            Mat l_object;
            controller->convertImage(QImage((localPath + "/dark/map/sample_" + QString::number(i) + ".png")), &l_object,&l_result);
            if(!l_result) {
                observer.value = l_result;
                observer.print = false;
                return;
            }
            controller->setMatObject(l_object,&l_result);
            if(!l_result) {
                observer.value = l_result;
                observer.print = false;
                return;
            }
            controller->setMask("dark/map/find_" + QString::number(i));
            controller->findObject(nullptr,&l_result);
            if(!l_result){
                observer.value = l_result;
                observer.print = false;
                return;
            }
            Mat temp = controller->cutImage();
            controller->compareObject(0.12,&temp,nullptr,&l_result);//0.12?
            if(l_result) return;
        }
        x++;
    }
    checkEndStage(&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
}

void Cathedral::checkEndStage(ErrorList *result){
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    controller->compareSample("dark/waypoints","sample_next","state_next",&l_result,true);
    if(l_result){
        controller->clickButton("dark/waypoints","button_next",&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        checkWarnings();
        return;
    }
    controller->compareSample("dark/waypoints","sample_end","state_end",&l_result);
    if(l_result) {
        controller->clickButton("dark/waypoints","button_end",&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        checkWarnings();
        currentStage = 4;
        return;
    }
}

void Cathedral::checkWaypoints(int &type, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    int x = 0;
    while (x < 10){
        controller->compareSample("battle/dark","sample","compare",&l_result,true); // для баттла 0.04? вроде как 0.006 хватает
        if(l_result) {
            type = 4;
            return;
        }
        for(int i = 0; i < 4; i++){
            controller->compareSample("dark/waypoints/" + nameWaypoints.at(i),"sample","compare",&l_result); // 0.065??
            if(l_result){
                type = i;
                return;
            }
        }
        Sleep(200);
        x++;
    }
    observer.value = l_result;
    observer.comment = "check waypoint";
    type = -1;
    return;
}

void Cathedral::attackWaypoints(int type, ErrorList *result, bool *battle) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    switch (type) {
    case 0: {//altar
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_move");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 1: {//camp
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_reward");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        checkWarnings();
        break;
    }
    case 2: {//game
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_skip");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 3: {//reflection
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 4: {//battle
        int x = 0;
        do {
            controller->Screenshot();
            controller->setMask("battle/dark/state_power",&l_result);
            if(!l_result) {
                observer.value = l_result;
                observer.print = false;
                return;
            }
            controller->findObject();
            checkPower(controller->cutImage(),&l_result);
            if(!l_result) {
                x++;
                Sleep(500);
            }
            else x = 3;
        } while(x < 3);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        x = 0;
        if(settings->premiumStatus){
            do{
                controller->clickButton("battle/dark","button_qstart",&l_result);
                if(l_result) break;
            }while(true);
            checkWarnings();
            checkBattleResult(battle);
            // while(x < 30){
            //     controller->compareSample("battle/end/quick","sample","compare",&l_result,true,0.025);
            //     if(!l_result) break;
            //     else {
            //         x++;
            //         Sleep(1000);
            //     }
            // }
            // if(x == 30){
            //     observer.value = l_result;
            //     observer.comment = "end battle";
            //     return;
            // }
            // controller->clickEsc();
            // ////?? надо ли, по-моему убрали их после битвы
            // controller->checkEvent(&l_result);
            // if(l_result) controller->skipEvent();
            // ////

        }
        else{
            //тут пока что пусто, лень
        }
        x = 0;
        while(x < 3){
            controller->compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
            if(l_result) break;
            x++;
            Sleep(500);
        }
        if(l_result) {
            controller->clickButton("dark/waypoints/blessing","button_" + QString::number(1 + rand() % 3));
            controller->clickButton("dark/waypoints/blessing","button_confirm");
        }
        else {
            observer.value = l_result;
            observer.comment = "blessing";
        }
        return;
    }
    default:{
        observer.value.warning = m_Warning::FAIL_CHECK;
        observer.comment = "wrong type of waypoint";
        return;
    }
    }
    do controller->compareSample("dark/waypoints","sample_complete","compare_complete",&l_result,true);
    while(!l_result);//подумать на будущее над счетчиком и если что проверку на ошибки выбрасывать
    controller->clickButton("dark/waypoints","button_close");
    return;
}

void Cathedral::fullGamePass(ErrorList *result) {
    emit controller->Logging("Начато полное прохождение собора");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, controller, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.");
        while(true) {
            findWaypoint(&l_result);
            if(!l_result) break;
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
        if(currentStage == 4) break;
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
        Mat l_object;
        controller->convertImage(QImage((localPath + "/dark/map/sample_0.png")), &l_object,&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.comment = ("convert->.../dark/map/sample_0.png");
            return;
        }
        controller->setMatObject(l_object,&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            return;
        }
        controller->setMask("dark/map/find_0");
        controller->findObject(nullptr,&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
        Mat tempMat = controller->cutImage();
        controller->compareObject(0.12,&tempMat,nullptr,&l_result);
        QVector<Rect> bossWay = getBossWay(controller->getRect());
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
                    i--;
                    errorCount++;
                    if(errorCount == 3) n = -5;
                    continue;
                }
                else errorCount = 0;
                attackWaypoints(type,nullptr,&battle);
                if(!battle) i--;//если поражение снова в ту точку
            }
            checkEndStage();
        }
        else{
            emit controller->Logging("Этаж был начат. Полное прохождение текущего этажа.");
            while(true) {
                findWaypoint(&l_result);
                if(!l_result) break;
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
        if(currentStage == 4) break;
        checkStage(&l_result);
        if(!l_result){
            observer.value = l_result;
            observer.print = false;
            return;
        }
    }
}

void Cathedral::checkBattleResult(bool *battle){
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    do controller->compareSample("battle/end/quick","sample","compare",&l_result,true,0.025);
    while (!l_result);
    controller->compareSample("battle/end/quick","sample_victory","state_victory",&l_result,true);
    if(!l_result) {
        controller->compareSample("battle/end/quick","sample_defeat","state_victory",&l_result,false);
        if(l_result) {
            if(battle) *battle = false;
        }
    }
    else {
        if(battle) *battle = true;
    }
    controller->clickEsc();
    ////?? надо ли, по-моему убрали их после битвы
    controller->checkEvent(&l_result);
    if(l_result) controller->skipEvent();
    ////
}

QVector<Rect> Cathedral::getBossWay(Rect &rect){
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

void Cathedral::safePower(ErrorList *result) {}
void Cathedral::setUnitsSet(ErrorList *result) {}
