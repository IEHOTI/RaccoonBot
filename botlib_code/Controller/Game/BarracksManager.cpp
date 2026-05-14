#include "Controller/Game/BarracksManager.h"
#include "CustomError/ErrorObserver.h"
#include "User/UserProfile.h"

#include <QThread>

BarracksManager::BarracksManager(ImageService& images, InputService& input,
                                 GameNavigator& navigator,
                                 LogFn log, LogMainFn logMain, RecognizeFn recognize, FixFn fix)
    : m_images(images)
    , m_input(input)
    , m_nav(navigator)
    , m_log(std::move(log))
    , m_logMain(std::move(logMain))
    , m_recognize(std::move(recognize))
    , m_fix(std::move(fix))
{
}


void BarracksManager::findBarracks(ErrorList* result)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.compareSample("squad/main", "sample", "compare", &l_result, true);
    if (!l_result) {
        observer.value = l_result;
        observer.comment = "not found barrack";
        return;
    }
}

void BarracksManager::entryBarracks(ErrorList* result)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    int x = 0;
    cv::Rect l_rect = m_images.getRect();
    do {
        m_input.clickPosition(l_rect);
        QThread::msleep(500);
        m_images.compareSample("squad/main", "sample_info", "compare_info", &l_result, true);
        if (!l_result) {
            m_images.compareSample("squad/main", "sample_info", "compare_pals", &l_result);
            if (!l_result) {
                x++;
                if (x == 10) {
                    m_fix();
                    x = 0;
                }
                QThread::msleep(2000);
            } else
                m_nav.clickButton("squad/main", "button_info");
        } else
            m_input.click();
    } while (!l_result);

    x = 0;
    do {
        m_images.compareSample("squad/main", "sample_barrack", "compare_barrack", &l_result, true);
        if (!l_result) {
            x++;
            if (x == 50) {
                m_fix();
                x = 0;
            } else
                QThread::msleep(500);
        }
    } while (!l_result);
}

void BarracksManager::scanSquadCount(userProfile* user, ErrorList* result)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};

    m_images.setMask("squad/main/count", &l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    m_images.findObject();
    cv::Mat temp;
    m_images.changeColor(m_images.cutImage(), &temp, &l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }

    int number = 0;
    m_recognize(temp, number);
    if (number < 2) {
        observer.value.warning = m_Warning::FAIL_RECOGNIZE;
        observer.comment = "unit count";
        return;
    }

    user->count_units = number;
    m_input.clickEsc();
    m_input.clickEsc();
}

void BarracksManager::setUnitSet(int index, typeSet set, ErrorList* result)
{
    if (set == typeSet::NOT_TOUCH)
        return;

    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    if (index < 0 || index > 8) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "unit index out of range";
        return;
    }

    const QString pagePath = "squad/main/unit";
    const QVector<int> unitPos = {400, 490, 330, 560, 280, 610, 240, 650};
    const int yPos = 320;

    int x = 0;
    do {
        m_images.compareSample(pagePath, "sample", "compare", &l_result, true);
        if (!l_result) {
            m_input.clickPosition(cv::Rect(unitPos[index], yPos, 0, 0), &l_result, 1);
            if (!l_result) {
                x++;
                if (x == 50) {
                    m_fix();
                    x = 0;
                }
                QThread::msleep(500);
            }
        }
    } while (!l_result);

    m_nav.openAnySets(&l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    x = 0;
    switch (set) {
    case typeSet::UNEQUIP:
        m_images.compareSample(pagePath, "sample_set_3", "state_unequip", &l_result);
        if (l_result)
            m_input.click();
        break;
    case typeSet::SET_1:
        m_images.compareSample(pagePath, "sample_set_0", "state_set", &l_result);
        if (l_result) {
            m_logMain("У " + QString::number(index + 1) + " бойца отсутсвуют наборы вещей.", false);
            break;
        }
        m_nav.clickButton(pagePath, "button_set_1");
        break;
    case typeSet::SET_2:
        m_images.compareSample(pagePath, "sample_set_0", "state_set", &l_result);
        if (l_result) {
            m_logMain("У " + QString::number(index + 1) + " бойца отсутсвует первый набор вещей.", false);
            break;
        }
        m_images.compareSample(pagePath, "sample_set_1", "state_set", &l_result);
        if (l_result) {
            m_logMain("У " + QString::number(index + 1) + " бойца отсутсвует второй набор вещей.", false);
            break;
        }
        m_nav.clickButton(pagePath, "button_set_2");
        break;
    case typeSet::SET_3:
        m_images.compareSample(pagePath, "sample_set_3", "state_set", &l_result);
        if (!l_result) {
            m_logMain("У " + QString::number(index + 1) + " бойца отсутсвует третий набор вещей.", false);
            break;
        }
        m_nav.clickButton(pagePath, "button_set_3");
        break;
    default:
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "unit set out of range";
        return;
    }

    while (true) {
        m_images.compareSample(pagePath, "sample_set", "compare_set", &l_result, true);
        if (l_result) {
            x++;
            if (x == 5) {
                m_input.clickEsc();
                break;
            }
            QThread::msleep(1000);
        } else {
            x = 0;
            do {
                m_images.compareSample(pagePath, "sample_confirm", "compare_confirm",
                                       &l_result, true);
                if (!l_result) {
                    x++;
                    if (x == 15) {
                        m_fix();
                        x = 0;
                    }
                }
                QThread::msleep(1000);
            } while (!l_result);

            m_nav.clickButton(pagePath, "button_confirm");
            x = 0;
            do {
                m_images.compareSample(pagePath, "confirm_warning", "compare_confirm",
                                       &l_result, true);
                if (l_result) {
                    m_nav.clickButton(pagePath, "button_yes");
                    x = 10;
                } else {
                    x++;
                    QThread::msleep(500);
                }
            } while (x < 10);
            break;
        }
    }

    x = 0;
    do {
        m_images.compareSample(pagePath, "sample", "compare", &l_result, true);
        if (!l_result) {
            x++;
            if (x == 50) {
                m_fix();
                x = 0;
            }
            QThread::msleep(500);
        }
    } while (!l_result);

    m_input.clickEsc();
}
