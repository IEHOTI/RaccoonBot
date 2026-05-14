#include "Controller/Game/UserManager.h"
#include "CustomError/ErrorObserver.h"
#include "User/UserProfile.h"

#include <QCoreApplication>
#include <QThread>

UserManager::UserManager(ImageService& images, GameNavigator& navigator,
                         LogFn log, RecognizeFn recognize, FixFn fix)
    : m_images(images)
    , m_nav(navigator)
    , m_log(std::move(log))
    , m_recognize(std::move(recognize))
    , m_fix(std::move(fix))
{
}


void UserManager::userInitialize(userProfile* user, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    do {
        m_nav.clickButton("main", "button_user", &l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    int x = 0;
    do {
        m_images.compareSample("user", "sample", "compare", &l_result, true);
        if (!l_result) {
            x++;
            QThread::msleep(1000);
        } else if (x == 10) {
            m_fix();
            x = 0;
        } else
            break;
    } while (x < 10);

    m_images.setMask("user/user_id", &l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    do {
        m_images.findObject(nullptr, &l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    m_recognize(m_images.cutImage(), user->user_ID);
    if (user->user_ID <= 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "scan user_id";
        return;
    }

    //////
    user->subscribe = typeSub::Sobor;
    //////

    m_images.setMask("user/user_power", &l_result);
    if (!l_result) NoPrintError(&observer, l_result);

    do {
        m_images.findObject(nullptr, &l_result);
        if (!l_result) m_fix();
    } while (!l_result);

    m_recognize(m_images.cutImage(), user->history_power);
    if (user->history_power <= 0) {
        observer.value.error = m_Error::FAIL_INIT;
        observer.comment = "scan user_power";
        return;
    }

    m_images.compareSample("user", "sample", "state_prem", &l_result);
    if (l_result) user->state_premium = false;
    else user->state_premium = true;

    m_images.compareSample("user", "sample", "state_ads", &l_result);
    if (l_result) user->state_ads = false;
    else user->state_ads = true;

    ////
    user->leftover_time = "9999999 days for Raccoons";
    ////

    m_nav.clickButton("user", "button_close");
}
