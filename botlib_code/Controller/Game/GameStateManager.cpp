#include "Controller/Game/GameStateManager.h"
#include "CustomError/ErrorObserver.h"
#include "CustomError/Exception.h"

#include <QCoreApplication>
#include <QThread>
#include <windows.h>

GameStateManager::GameStateManager(ControllerState& state, ImageService& images, InputService& input,
                                   GameNavigator& navigator, LogFn log, FixFn fix)
    : m_state(state)
    , m_images(images)
    , m_input(input)
    , m_nav(navigator)
    , m_log(std::move(log))
    , m_fix(std::move(fix))
{
}

// ---- Emulator validity ------------------------------------------------------

void GameStateManager::isEmpty(ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    if (m_state.main == NULL || m_state.game == NULL) {
        observer.value.error = m_Error::NO_ACTIVE_EMULATOR;
        return;
    }
}

void GameStateManager::isValidSize(ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    RECT temp;
    int width, height;
    GetWindowRect(m_state.game, &temp);
    width  = temp.right  - temp.left;
    height = temp.bottom - temp.top;
    if (width != 900 || height != 600) {
        observer.value.warning = m_Warning::WRONG_EMULATOR_SIZE;
        return;
    }
}

void GameStateManager::isValidPos(ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    RECT temp;
    GetWindowRect(m_state.game, &temp);
    if (temp.left != 1 || temp.top != 33) {
        observer.value.warning = m_Warning::WRONG_EMULATOR_POS;
        return;
    }
}

// ---- Loading waits ----------------------------------------------------------

void GameStateManager::checkLoading()
{
    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    do {
        m_images.compareSample("load", "sample", "compare", &l_result, true);
        if (!l_result)
            break;
        else
            QThread::msleep(1000);
    } while (true);
    QThread::msleep(500);
}

void GameStateManager::checkGameLoading()
{
    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    do {
        m_images.compareSample("load", "sample_open", "compare_open", &l_result, true);
        if (!l_result)
            break;
        else
            QThread::msleep(1000);
    } while (true);
    do {
        m_images.compareSample("load", "sample_logo", "compare_logo", &l_result, true);
        if (!l_result)
            break;
        else
            QThread::msleep(1000);
    } while (true);
    checkLoading();
}

// ---- Page checks ------------------------------------------------------------

void GameStateManager::skipEvent()
{
    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    do {
        checkEvent(&l_result);
        if (l_result) {
            m_input.clickEsc();
            QThread::msleep(2000);
        }
    } while (l_result);
}

void GameStateManager::checkPreMainPage()
{
    checkGameLoading();

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    checkEvent(&l_result);
    if (l_result)
        skipEvent();

    m_images.compareSample("load", "sample_mail", "compare_mail", &l_result, true);
    if (l_result) {
        m_nav.clickButton("load", "button_close_mail", &l_result);
        QThread::msleep(500);
    }

    do {
        m_images.compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if (l_result) {
            m_input.clickEsc();
            QThread::msleep(3000);
        }
    } while (l_result);

    do {
        m_images.compareSample("load", "sample_offer", "compare_offer", &l_result, true);
        if (l_result) {
            m_input.clickEsc();
            QThread::msleep(3000);
        }
    } while (l_result);

    do {
        m_images.compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if (l_result) {
            m_input.clickEsc();
            QThread::msleep(3000);
        }
    } while (l_result);
}

void GameStateManager::checkMainPage(ErrorList* result)
{
    QThread::msleep(500);
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.compareSample("main", "sample", "compare", &l_result, true);
    do {
        if (!l_result) {
            m_log("Первая проверка провалена.");
            m_images.compareSample("main", "sample", "compare_1", &l_result, true);
            if (!l_result) m_fix();
        }
    } while (!l_result);
}

void GameStateManager::checkEvent(ErrorList* result)
{
    m_log("Check Event");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.setMask("event/compare", &l_result);

    m_images.setMatObject("event/sample", &l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    m_images.findObject();
    cv::Mat find = m_images.cutImage();

    int x = 0;
    do {
        m_images.Screenshot();
        cv::Mat temp = m_images.getMatObject();
        m_images.compareObject(0.037, &find, &temp, &l_result);
        if (l_result)
            break;
        else {
            x++;
            QThread::msleep(500);
        }
    } while (x < 2);

    if (x == 2) {
        observer.value.warning = m_Warning::NO_EVENT;
        observer.print = false;
        return;
    }
}

void GameStateManager::checkSettings(ErrorList* result)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_nav.clickButton("main", "button_settings", &l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    int x = 0;
    do {
        m_images.compareSample("settings", "sample", "compare", &l_result, true);
        if (l_result)
            break;
        else {
            x++;
            QThread::msleep(500);
        }
    } while (x < 10);
    if (!l_result) NoPrintError(&observer, l_result);

    m_images.compareSample("settings", "sample", "state_fps", &l_result);
    if (!l_result)
        m_nav.clickButton("settings", "button_fps", &l_result);

    if (!l_result) m_fix();

    m_images.compareSample("settings", "sample", "state_lang", &l_result);
    while (!l_result) {
        m_nav.clickButton("settings", "button_lang", &l_result);
        if (!l_result) m_fix();
    }
    QThread::msleep(1000);
    while (!l_result) {
        m_images.compareSample("settings", "sample_change_lang", "compare_change_lang",
                               &l_result, true);
        if (!l_result) m_fix();
    }
    while (!l_result) {
        m_nav.clickButton("settings", "button_en", &l_result);
        if (!l_result) m_fix();
    }
    while (!l_result) {
        m_images.compareSample("settings", "sample_confirm", "compare_confirm", &l_result, true);
        if (!l_result) m_fix();
    }
    while (!l_result) {
        m_nav.clickButton("settings", "button_yes", &l_result);
        if (!l_result) m_fix();
    }
    m_input.clickEsc(nullptr, 2);
}
