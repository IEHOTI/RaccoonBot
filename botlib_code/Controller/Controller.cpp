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
    if(lib) delete lib;
    lib = nullptr;
}

void Controller::Start(userProfile *user, ErrorList *result) {
    isWorking = true;
    emit Logging("Бот запущен");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    try {
        if(!lib) lib = new ImageLibrary("ImageLib.dll");

        bool result = false;
        lib->load(result);
        if(!result) throw ImageException("Cannot load images");
        // else {
        //     QStringList allImages;
        //     lib->getList(allImages);
        //     qDebug() << allImages;
        //     cv::Mat temp;
        //     lib->get(":/pages/load/compare_open.png",temp);
        //     if(temp.empty()) throw ImageException("empty img");
        // }

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
            if(!l_result){
                fixGameError(&l_result);
                if(!l_result){
                    observer.value.error = m_Error::FAIL_INIT;
                    observer.comment = "find mainPage";
                    isWorking = false;
                    return;
                }
                else l_result.warning = m_Warning::UNKNOWN;
            }
        }while(!l_result);
        do{
            checkSettings(&l_result);
            if(!l_result){
                fixGameError(&l_result);
                if(!l_result){
                    observer.value.error = m_Error::FAIL_INIT;
                    observer.comment = "find settings";
                    isWorking = false;
                    return;
                }
                else l_result.warning = m_Warning::UNKNOWN;
            }
        }while(!l_result);
        userInitialize(user,&l_result);
        if(!l_result) {
            observer.value.error = m_Error::FAIL_INIT;
            observer.print = false;
            isWorking = false;
            return;
        }
        refreshMainPage(&l_result);
        if(!l_result) {
            observer.value = l_result;
            observer.print = false;
            isWorking = false;
            return;
        }
        findBarracks(&l_result);
        if(l_result) {
            entryBarracks(&l_result);
            if(l_result){
                scanSquadCount(user,&l_result);
                if(!l_result) {
                    observer.value = l_result;
                    observer.print = false;
                    isWorking = false;
                    return;
                }
            }
            else {
                observer.value = l_result;
                observer.print = false;
                isWorking = false;
                return;
            }
        }
        else {
            observer.value = l_result;
            observer.print = false;
            isWorking = false;
            return;
        }
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
