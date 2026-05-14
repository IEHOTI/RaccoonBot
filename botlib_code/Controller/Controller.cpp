#include "Controller/Controller.h"

#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"
#include "Emulators/Core/EmulatorComponents.h"
#include "ImageLibrary/ImageLibrary.h"
#include "User/UserProfile.h"

// tmp
#include "Emulators/LDPlayer/ldplayer.h"
//

#include <QCoreApplication>
#include <QThread>

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

Controller::Controller(QObject* parent)
    : QObject(parent)
    // Build each service in dependency order, binding logging/signal callbacks via lambdas.
    , m_images(m_state, [this](const QString& msg) { LocalLogging(msg); })
    , m_input(m_state, m_images, [this](const QString& msg) { LocalLogging(msg); })
    , m_nav(m_images, m_input, [this](const QString& msg) { LocalLogging(msg); },
            [this]() { fixErrors(); })
    , m_checker(m_state, m_images, m_input, m_nav,
                [this](const QString& msg) { LocalLogging(msg); },
                [this]() { fixErrors(); })
    , m_fixer(m_images, m_input, m_nav, m_checker,
              [this](const QString& msg) { LocalLogging(msg); },
              [this](const QString& msg, bool print) { emit Logging(msg, print); })
    , m_hero(m_images, m_input, m_nav,
             [this](const QString& msg) { LocalLogging(msg); },
             [this](const QString& msg, bool print) { emit Logging(msg, print); },
             [this]() { fixErrors(); })
    , m_barracks(m_images, m_input, m_nav,
                 [this](const QString& msg) { LocalLogging(msg); },
                 [this](const QString& msg, bool print) { emit Logging(msg, print); },
                 [this](const cv::Mat& mat, int& n) { emit Recognize(mat, n); },
                 [this]() { fixErrors(); })
    , m_user(m_images, m_nav,
             [this](const QString& msg) { LocalLogging(msg); },
             [this](const cv::Mat& mat, int& n) { emit Recognize(mat, n); },
             [this]() { fixErrors(); })
{
}

Controller::~Controller()
{
    m_state.lib = nullptr;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void Controller::Start(userProfile* user, ErrorList* result)
{
    m_state.isWorking = true;
    emit Logging("Бот запущен");

    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};

    if (!m_state.lib) {
        emit Logging("Ожидание прогрева бота...");
        while (!m_state.lib) {
            LocalLogging("+++++++++Загрузка...++++++++++++++");
            QThread::msleep(500);
        }
    }
    emit Logging("Начало работы");

    int x = 0;
    while (x < 3) {
    try {
        if (!FindEmulator(user->emulator_name, &m_state.main, &m_state.game)) {
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "find emulator";
            return;
        }
        QCoreApplication::processEvents();

        char name[256];
        GetClassNameA(m_state.main, name, sizeof(name));
        if (strcmp(name, "LDPlayerMainFrame") == 0) {
            user->emulatorType = typeEmu::ld_player;
            Emulator* emulator = new LDPlayer(this);
            emulator->Initialize(&m_state.main);
            emit emulatorCreated(emulator);
        } else {
            observer.value.error = m_Error::FAIL_INIT;
            observer.comment = "recognize emulator type";
            m_state.isWorking = false;
            return;
        }
        QCoreApplication::processEvents();

        do {
            m_checker.checkPreMainPage();
            m_checker.checkMainPage(&l_result);
            if (!l_result) fixErrors();
        } while (!l_result);

        do {
            m_checker.checkSettings(&l_result);
            if (!l_result) fixErrors();
        } while (!l_result);

        m_user.userInitialize(user, &l_result);
        if (!l_result) fixErrors();

        QThread::msleep(1500);

        m_barracks.findBarracks(&l_result);
        if (!l_result) {
            m_fixer.refreshMainPage(&l_result);
            if (!l_result) fixErrors();
            m_barracks.findBarracks(&l_result);
        }

        if (l_result) {
            m_barracks.entryBarracks(&l_result);
            if (l_result) {
                m_barracks.scanSquadCount(user, &l_result);
                if (!l_result) fixErrors();
            } else
                fixErrors();
        } else
            fixErrors();

        m_state.isWorking = false;
        emit endStart();
        return;

    } catch (const StopException& e) {
        observer.value.error = m_Error::STOP_TASK;
        observer.comment = e.what();
        emit Logging("Бот остановлен");
        CleanUp();
        m_state.isWorking = false;
        QThread::currentThread()->quit();
        return;
    } catch (const PauseException& e) {
        observer.value.error = m_Error::PAUSE_TASK;
        observer.comment = e.what();
        return;
    } catch (const ImageException& e) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = e.what();
        emit Logging("Бот остановлен");
        CleanUp();
        m_state.isWorking = false;
        QThread::currentThread()->quit();
        return;
    } catch (const FixerException& e) {
        LocalLogging(QString("Controller ") + e.what());
        if (e.refreshEmulator()) {
            emit Logging("Перезагрузка эмулятора");
            emit emulatorRefresh();
        } else
            emit Logging("Устранение проблем...");
    }
    ++x;
    }
    Stop();
}

void Controller::Stop()
{
    if (m_state.isWorking)
        throw StopException();
    else
        QThread::currentThread()->quit();
}

void Controller::CleanUp()
{
    m_state.main = 0;
    m_state.game = 0;
    m_state.object.release();
    m_state.mask.release();
    m_state.sample.release();
    m_state.rect = {0, 0, 0, 0};
}

void Controller::imageLoader(ImageLibrary *library) { m_state.lib = library; }

void Controller::LocalLogging(const QString& msg)
{
    QCoreApplication::processEvents();
    emit errorLogging(msg);
}

// ---------------------------------------------------------------------------
// Config
// ---------------------------------------------------------------------------

void Controller::setMainPath(const QString& path)
{
    m_state.mainPath = path;
}

QString& Controller::getMainPath()
{
    return m_state.mainPath;
}

cv::Rect& Controller::getRect()
{
    return m_state.rect;
}

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

void Controller::getGameError()
{
    if (m_state.object.empty())
        emit errorLogging("Пустой m_object");
    else
        imshow("1", m_state.object);

    if (m_state.mask.empty())
        emit errorLogging("Пустой m_mask");
    else
        imshow("2", m_state.mask);

    if (m_state.sample.empty())
        emit errorLogging("Пустой m_sample");
    else
        imshow("3", m_state.sample);
}

// ---------------------------------------------------------------------------
// Delegation wrappers — keep the public API identical to the original Controller
// ---------------------------------------------------------------------------

cv::Mat Controller::cutImage()                        { return m_images.cutImage(); }
void Controller::changeColor(const cv::Mat& b, cv::Mat* a, ErrorList* r, bool c)
                                                       { m_images.changeColor(b, a, r, c); }
void Controller::findObject(const cv::Mat* f, ErrorList* r)
                                                       { m_images.findObject(f, r); }
void Controller::compareObject(double rv, const cv::Mat* o, const cv::Mat* s, ErrorList* r)
                                                       { m_images.compareObject(rv, o, s, r); }
void Controller::compareSample(const QString& pp, const QString& sp, const QString& mp,
                                ErrorList* r, bool sh, double rv)
                                                       { m_images.compareSample(pp, sp, mp, r, sh, rv); }
void Controller::Screenshot()                         { m_images.Screenshot(); }
void Controller::saveImage(const QString& p, const cv::Mat& img, ErrorList* r)
                                                       { m_images.saveImage(p, img, r); }
void Controller::setMatObject(const QString& img, ErrorList* r) { m_images.setMatObject(img, r); }
void Controller::setMatObject(const cv::Mat& img, ErrorList* r) { m_images.setMatObject(img, r); }
void Controller::setSample(const cv::Mat& s, ErrorList* r)     { m_images.setSample(s, r); }
void Controller::setSample(const QString& s, ErrorList* r)     { m_images.setSample(s, r); }
void Controller::setMask(const cv::Mat& m, ErrorList* r)       { m_images.setMask(m, r); }
void Controller::setMask(const QString& m, ErrorList* r)       { m_images.setMask(m, r); }
cv::Mat Controller::getMatObject()                    { return m_images.getMatObject(); }
void Controller::getImageFromLibrary(const QString& p, cv::Mat& r)
                                                       { m_images.getImageFromLibrary(p, r); }

void Controller::clickEsc(ErrorList* r, int c)        { m_input.clickEsc(r, c); }
void Controller::clickReturn(ErrorList* r, int c)     { m_input.clickReturn(r, c); }
void Controller::click(ErrorList* r, int c, int d)    { m_input.click(r, c, d); }
void Controller::clickPosition(const cv::Rect& p, ErrorList* r, int c, int d)
                                                       { m_input.clickPosition(p, r, c, d); }
void Controller::clickSwipe(const cv::Rect& s, const cv::Rect& f, ErrorList* r)
                                                       { m_input.clickSwipe(s, f, r); }

void Controller::isEmpty(ErrorList* r)                { m_checker.isEmpty(r); }
void Controller::isValidSize(ErrorList* r)            { m_checker.isValidSize(r); }
void Controller::isValidPos(ErrorList* r)             { m_checker.isValidPos(r); }
void Controller::checkLoading()                       { m_checker.checkLoading(); }
void Controller::checkGameLoading()                   { m_checker.checkGameLoading(); }
void Controller::checkPreMainPage()                   { m_checker.checkPreMainPage(); }
void Controller::checkMainPage(ErrorList* r)          { m_checker.checkMainPage(r); }
void Controller::checkEvent(ErrorList* r)             { m_checker.checkEvent(r); }
void Controller::checkSettings(ErrorList* r)          { m_checker.checkSettings(r); }
void Controller::skipEvent()                          { m_checker.skipEvent(); }

void Controller::checkMap(ErrorList* r)               { m_nav.checkMap(r); }
void Controller::clickButton(const QString& pp, const QString& bn, ErrorList* r, int c, int d)
                                                       { m_nav.clickButton(pp, bn, r, c, d); }
void Controller::clickMapButton(const QString& pn, const QString& bn, ErrorList* r, int c, int d)
                                                       { m_nav.clickMapButton(pn, bn, r, c, d); }
void Controller::openAnySets(ErrorList* r)            { m_nav.openAnySets(r); }

void Controller::fixGameError(ErrorList* r)           { m_fixer.fixGameError(r); }
void Controller::fixPopUpError(ErrorList* r)          { m_fixer.fixPopUpError(r); }
void Controller::fixBattleSettings()                  { m_fixer.fixBattleSettings(); }
void Controller::refreshMainPage(ErrorList* r)        { m_fixer.refreshMainPage(r); }
void Controller::openHeroPage(ErrorList* r)           { m_hero.openHeroPage(r); }
void Controller::setHeroSet(typeSet s, ErrorList* r)  { m_hero.setHeroSet(s, r); }
void Controller::setHeroRelics(ErrorList* r, const QList<QString>* l)
                                                       { m_hero.setHeroRelics(r, l); }

void Controller::findBarracks(ErrorList* r)           { m_barracks.findBarracks(r); }
void Controller::entryBarracks(ErrorList* r)          { m_barracks.entryBarracks(r); }
void Controller::scanSquadCount(userProfile* u, ErrorList* r)
                                                       { m_barracks.scanSquadCount(u, r); }
void Controller::setUnitSet(int i, typeSet s, ErrorList* r)
                                                       { m_barracks.setUnitSet(i, s, r); }

void Controller::userInitialize(userProfile* u, ErrorList* r)
                                                       { m_user.userInitialize(u, r); }
