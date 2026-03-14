#include "Controller/Controller.h"
//обманываю линковщик

#include "User/UserProfile.h"
#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"
#include "Emulators/Core/EmulatorComponents.h"
#include "ImageLibrary/ImageLibrary.h"

//tmp
#include "Emulators/LDPlayer/ldplayer.h"
//

#include <QCoreApplication>
#include <QThread>

Controller::Controller(QObject *parent) : QObject(parent) {
    m_main = 0;
    m_game = 0;
    m_rect = { 0,0,0,0 };
    mainPath = ":/pages";
    isWorking = false;
    lib = nullptr;
}

Controller::~Controller() {
    lib = nullptr;
}

void Controller::Start(userProfile *user, ErrorList *result) {
    isWorking = true;
    emit Logging("Бот запущен");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    try {
        if(!lib) {
            Logging("Ожидание прогрева бота...");
            while(!lib) {
                LocalLogging("+++++++++Загрузка...++++++++++++++");
                QThread::msleep(500);
            }
        }
        Logging("Начало работы");
        if(!FindEmulator(user->emulator_name,&m_main,&m_game)) {
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "find emulator";
            return;
        }
        QCoreApplication::processEvents();
        char name[256];
        GetClassNameA(m_main, name, sizeof(name));
        //вот это вынести в отдельную функцию в emulators components
        if (strcmp(name, "LDPlayerMainFrame") == 0) {
            user->emulatorType = typeEmu::ld_player;
            Emulator *emulator = new LDPlayer(this);
            emulator->Initialize(&m_main);
            emit emulatorCreated(emulator);
        }
        else {
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "recognize emulator type";
            isWorking = false;
            return;
        }
        //
        QCoreApplication::processEvents();
        do{
            checkPreMainPage();
            checkMainPage(&l_result);
            if(!l_result) fixErrors();
        }while(!l_result);
        do{
            checkSettings(&l_result);
            if(!l_result) fixErrors();
        }while(!l_result);

        userInitialize(user,&l_result);
        if(!l_result) fixErrors();

        findBarracks(&l_result);
        if(!l_result) {
            refreshMainPage(&l_result);
            if(!l_result) fixErrors();
            findBarracks(&l_result);
        }

        if(l_result) {
            entryBarracks(&l_result);
            if(l_result){
                scanSquadCount(user,&l_result);
                if(!l_result) fixErrors();
            }
            else fixErrors();
        }
        else fixErrors();
        isWorking = false;
        emit endStart();
    } catch (const StopException &e) {
        observer.value.error = m_Error::STOP_TASK;
        observer.comment = e.what();
        Logging("Бот остановлен");
        CleanUp();
        isWorking = false;
        QThread::currentThread()->quit();
        return;
    } catch (const PauseException &e) {
        observer.value.error = m_Error::PAUSE_TASK;
        observer.comment = e.what();
        return;
    } catch (const ImageException &e) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = e.what();
        Logging("Бот остановлен");
        CleanUp();
        isWorking = false;
        QThread::currentThread()->quit();
        return;
    }
    catch (const FixerException &e) {
        LocalLogging(e.what());
        if(e.refreshEmulator()) {
            Logging("Перезагрузка эмулятора");
            emit emulatorRefresh();
        }
        observer.comment = e.what();
        CleanUp();
        isWorking = false;
        QThread::currentThread()->quit();
        return;
    }
}

void Controller::Stop() {
    if(isWorking) throw StopException();
    else QThread::currentThread()->quit();
}

void Controller::CleanUp(){
    m_main = 0;
    m_game = 0;
    m_object.release();
    m_mask.release();
    m_sample.release();
    m_rect = {0,0,0,0};
}

void Controller::LocalLogging(const QString &msg) {
    //getGameError();
    QCoreApplication::processEvents();
    emit errorLogging(msg);
}
