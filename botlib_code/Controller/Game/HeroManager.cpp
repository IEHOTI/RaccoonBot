#include "Controller/Game/HeroManager.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

HeroManager::HeroManager(ImageService& images, InputService& input,
                         GameNavigator& navigator, LogFn log, LogMainFn logMain, FixFn fix)
    : m_images(images)
    , m_input(input)
    , m_nav(navigator)
    , m_log(std::move(log))
    , m_logMain(std::move(logMain))
    , m_fix(std::move(fix))
{
}

void HeroManager::openHeroPage(ErrorList* result)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    do {
        m_nav.clickButton("main", "button_user", &l_result, 3);
        if (!l_result) m_fix();
    } while (!l_result);

    int x = 0;
    do {
        m_images.compareSample("user", "sample", "compare", &l_result, true);
        if (!l_result) {
            x++;
            if (x == 10) {
                m_fix();
                x = 0;
            } else
                QThread::msleep(1000);
        }
    } while (!l_result);

    m_nav.clickButton("user", "button_battles");

    do {
        m_images.compareSample("user", "sample_battles", "compare_battles", &l_result, true);
        if (!l_result) m_fix();
    } while (!l_result);

    m_nav.clickButton("user", "button_hero");
    QThread::msleep(500);
    do {
        m_images.compareSample("hero", "sample", "compare", &l_result, true);
        if (!l_result) m_fix();
    } while (!l_result);
}

void HeroManager::setHeroSet(typeSet set, ErrorList* result)
{
    if (set == typeSet::NOT_TOUCH)
        return;

    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};

    do {
        openHeroPage(&l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    do {
        m_nav.openAnySets(&l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    const QString pagePath = "squad/main/unit";
    switch (set) {
    case typeSet::UNEQUIP:
        m_images.compareSample(pagePath, "sample_set_3", "state_unequip", &l_result);
        if (l_result)
            m_input.click();
        break;
    case typeSet::SET_1:
        m_images.compareSample(pagePath, "sample_set_0", "state_set", &l_result);
        if (l_result) {
            m_logMain("У героя отсутствуют наборы вещей.", false);
            break;
        }
        m_nav.clickButton("hero", "button_set_1");
        break;
    case typeSet::SET_2:
        m_images.compareSample(pagePath, "sample_set_0", "state_set", &l_result);
        if (l_result) {
            m_logMain("У героя отсутствует первый набор вещей.", false);
            break;
        }
        m_images.compareSample(pagePath, "sample_set_1", "state_set", &l_result);
        if (l_result) {
            m_logMain("У героя отсутствует второй набор вещей.", false);
            break;
        }
        m_nav.clickButton("hero", "button_set_2");
        break;
    case typeSet::SET_3:
        m_images.compareSample(pagePath, "sample_set_3", "state_set", &l_result);
        if (!l_result) {
            m_logMain("У героя отсутствует третий набор вещей.", false);
            break;
        }
        m_nav.clickButton("hero", "button_set_3");
        break;
    default:
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "hero set out of range";
        return;
    }

    do {
        m_images.compareSample("hero", "sample_confirm_set", "compare_confirm_set",
                               &l_result, true);
        if (!l_result) m_fix();
    } while (!l_result);

    m_nav.clickButton("hero", "button_confirm_set");
    m_input.clickEsc();
}

void HeroManager::setHeroRelics(ErrorList* result, const QList<QString>* relicsList)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};

    if (relicsList->size() < 0 || relicsList->size() > 5) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "setHeroRelics->oversize list";
        return;
    }

    do {
        openHeroPage(&l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    do {
        m_nav.openAnySets(&l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    do {
        m_images.compareSample("squad/main/unit", "sample_set_3", "state_unequip",
                               &l_result, true);
        if (!l_result) m_fix();
    } while (!l_result);
    m_input.click();
    m_input.clickEsc();

    do {
        m_images.compareSample("hero", "sample_empty", "compare_empty", &l_result, true);
        if (!l_result) m_fix();
    } while (!l_result);

    QList<QString> temp = relicsList->toList();

    for (int count = 1; count < 9; count++) {
        m_images.Screenshot();
        m_images.setSample(m_images.getMatObject());
        for (int i = 0, n = temp.size(); i < n; ++i) {
            QString tempString = temp[i];
            m_images.setMatObject("hero/" + tempString);
            m_images.compareObject(0.02, nullptr, nullptr, &l_result);
            if (l_result) {
                m_input.click();
                QThread::msleep(500);
                do {
                    m_images.compareSample("hero", "sample_equip_relic", "compare_equip_relic",
                                           &l_result, true);
                    if (!l_result) m_fix();
                } while (!l_result);
                m_nav.clickButton("hero", "button_equip_relic");
                temp.removeOne(tempString);
                --i;
                --n;
            }
        }
        m_input.clickSwipe({680, 545, 0, 0}, {680, 330, 0, 0});
    }
    m_input.clickEsc();
}
