#include "Controller/Game/GameNavigator.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

GameNavigator::GameNavigator(ImageService& images, InputService& input, LogFn log, FixFn fix)
    : m_images(images)
    , m_input(input)
    , m_log(std::move(log))
    , m_fix(std::move(fix))
{
}

void GameNavigator::checkMap(ErrorList* result)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    int x = 0;
    while (true) {
        m_images.compareSample("map", "sample", "compare", &l_result, true);
        if (!l_result) {
            if (++x > 15) m_fix();
            else QThread::msleep(1000);
        } else
            return;
    }
}

void GameNavigator::clickButton(const QString& pagePath, const QString& buttonName,
                                ErrorList* result, int count, int delay)
{
    m_log("=Клик по кнопке:\"" + m_images.getMainPath() + "/" + pagePath + "/" + buttonName + ".png\"");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};

    m_images.setMask(pagePath + "/" + buttonName, &l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    m_images.findObject(nullptr, &l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    m_input.click(result, count, delay);
}

void GameNavigator::clickMapButton(const QString& pageName, const QString& buttonName,
                                   ErrorList* result, int count, int delay)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    do {
        checkMap(&l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    cv::Rect swipeRight = {690, 290, 0, 0};
    cv::Rect swipeLeft  = {100, 290, 0, 0};

    int x = 0;
    while (x < 3) {
        m_images.compareSample("map", pageName, buttonName, &l_result, true);
        if (!l_result) {
            m_input.clickSwipe(swipeRight, swipeLeft);
            x++;
        } else
            break;
    }
    if (!l_result) {
        x = 0;
        while (x < 3) {
            m_images.compareSample("map", pageName, buttonName, &l_result, true);
            if (!l_result) {
                m_input.clickSwipe(swipeLeft, swipeRight);
                x++;
            } else
                break;
        }
    }
    if (!l_result) {
        observer.value.warning = m_Warning::FAIL_PAGE;
        observer.comment = "Cant find " + buttonName;
        return;
    }

    m_input.click(result, count, delay);
}

void GameNavigator::openAnySets(ErrorList* result)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    const QString pagePath = "squad/main/unit";
    int x = 0;
    do {
        m_images.compareSample(pagePath, "sample_set", "compare_set", &l_result, true);
        if (!l_result) {
            m_images.compareSample(pagePath, "sample", "button_set", &l_result);
            if (!l_result) {
                x++;
                QThread::msleep(1000);
                if (x == 10) {
                    m_fix();
                    x = 0;
                }
            } else
                m_input.click(&l_result, 1);
        }
    } while (!l_result);
}
