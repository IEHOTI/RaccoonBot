#include "Controller/Game/GameFixer.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

GameFixer::GameFixer(ImageService& images, InputService& input,
                     GameNavigator& navigator, GameStateManager& checker,
                     LogFn log, LogMainFn logMain)
    : m_images(images)
    , m_input(input)
    , m_nav(navigator)
    , m_checker(checker)
    , m_log(std::move(log))
    , m_logMain(std::move(logMain))
{
}

void GameFixer::fixPopUpError(ErrorList* result)
{
    m_log("===Фикс всплывающих окон===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    QList<QString> types = {"device", "sleep", "daily"};

    for (const QString& type : types) {
        m_images.compareSample("warnings/general", "sample_" + type,
                               "compare_" + type, &l_result, true);
        if (l_result) {
            if (type == "device") {
                m_logMain("Вход с другого устройства", true);
                m_logMain("Ожидание 30 минут", true);
                QThread::sleep(1800);
                m_logMain("Продолжаем работу.", true);
            }
            m_nav.clickButton("warnings/general", "button_" + type, &l_result, 2);
            if (!l_result) {
                observer.value = l_result;
                observer.print = false;
                return;
            }
            m_images.compareSample("load", "sample", "compare", &l_result, true);
            if (l_result) {
                m_checker.checkLoading();
                return;
            }
        }
    }

    QThread::msleep(1000);
    m_images.compareSample("warnings/general", "sample_goblin", "compare_goblin", &l_result, true);
    if (l_result)
        m_input.clickEsc();
}

void GameFixer::fixGameError(ErrorList* result)
{
    if (result && *result)
        return;

    m_log("===Фикс полный===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    fixPopUpError(&l_result);
    if (l_result)
        return;

    do {
        m_images.compareSample("load", "sample", "compare", &l_result, true);
    } while (!l_result);

    m_checker.checkLoading();
    m_checker.checkPreMainPage();
    m_checker.checkMainPage();
}

void GameFixer::fixBattleSettings()
{
    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    do {
        m_images.compareSample("load", "sample", "compare", &l_result, true);
        if (l_result)
            break;
        else
            QThread::msleep(1000);
    } while (true);

    m_checker.checkLoading();

    m_images.compareSample("battle/general", "sample", "compare_hero", &l_result, true);
    if (l_result)
        m_input.click();
    m_images.compareSample("battle/general", "sample", "compare_damage", &l_result);
    if (l_result)
        m_input.click();
    m_images.compareSample("battle/general", "sample", "compare_effects", &l_result);
    if (l_result)
        m_input.click();

    do {
        m_images.compareSample("battle/general", "sample", "compare_speed", &l_result, true);
        if (!l_result) {
            m_nav.clickButton("battle/general", "button_speed");
            QThread::msleep(500);
        }
    } while (!l_result);
}

void GameFixer::refreshMainPage(ErrorList* result)
{
    m_log("===Обновление главной страницы===");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};

    m_checker.checkMainPage(&l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }

    m_nav.clickButton("main", "button_friends", &l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }

    int x = 0;
    do {
        m_images.compareSample("top_players", "sample", "compare", &l_result, true);
        if (l_result)
            break;
        else
            x++;
        QThread::msleep(500);
    } while (x < 100);
    if (x == 100) {
        observer.value.warning = m_Warning::FAIL_COMPARE;
        observer.comment = "top_players";
        return;
    }

    do {
        m_images.compareSample("top_players", "sample_visit", "compare_visit", &l_result, true);
        if (l_result)
            m_input.click();
        else {
            m_input.clickPosition(cv::Rect(480, 200, 0, 0));
            QThread::msleep(500);
            m_images.compareSample("load", "sample", "compare", &l_result, true);
        }
    } while (!l_result);

    do {
        m_images.compareSample("top_players", "sample_top", "compare_top", &l_result, true);
        if (!l_result)
            QThread::msleep(500);
    } while (!l_result);

    m_input.clickEsc();
    QThread::msleep(500);
    m_checker.checkLoading();

    m_checker.checkMainPage(&l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.print = false;
    }
}
