#include "Controller/Core/InputService.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>
#include <QCoreApplication>

InputService::InputService(ControllerState& state, ImageService& images, LogFn log)
    : m_state(state)
    , m_images(images)
    , m_log(std::move(log))
{
}

void InputService::focusWindow()
{
    SendMessage(m_state.main, WM_SETFOCUS, 0, 0);
    QThread::msleep(15);
    SendMessage(m_state.main, WM_MOUSEACTIVATE, (WPARAM)m_state.main,
                MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    SendMessage(m_state.game, WM_SETCURSOR, (WPARAM)m_state.game,
                MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
}

void InputService::clickEsc(ErrorList* result, int count)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.Screenshot();
    cv::Mat before, after;
    m_images.getMatObject().copyTo(before);

    focusWindow();

    int x = 0;
    while (true) {
        PostMessage(m_state.game, WM_KEYDOWN, VK_ESCAPE,
                    MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
        QThread::msleep(100);
        PostMessage(m_state.game, WM_KEYUP, VK_ESCAPE,
                    MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
        QThread::msleep(15);
        SendMessage(m_state.game, WM_SETCURSOR, (WPARAM)m_state.game,
                    MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
        QThread::msleep(65);

        m_images.Screenshot();
        m_images.getMatObject().copyTo(after);
        m_images.compareObject(0, &after, &before, &l_result);

        if (!l_result)
            return;
        if (++x > count) {
            observer.value.warning = m_Warning::FAIL_CLICK;
            observer.comment = "click Esc";
            return;
        }
    }
}

void InputService::clickReturn(ErrorList* result, int count)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.Screenshot();
    cv::Mat before, after;
    m_images.getMatObject().copyTo(before);

    focusWindow();

    int x = 0;
    while (true) {
        PostMessage(m_state.game, WM_KEYDOWN, VK_RETURN, 0);
        QThread::msleep(100);
        PostMessage(m_state.game, WM_KEYUP, VK_RETURN, 0);
        QThread::msleep(15);
        SendMessage(m_state.game, WM_SETCURSOR, (WPARAM)m_state.game,
                    MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
        QThread::msleep(65);

        m_images.Screenshot();
        m_images.getMatObject().copyTo(after);
        m_images.compareObject(0, &after, &before, &l_result);

        if (!l_result)
            return;
        if (++x > count) {
            observer.value.warning = m_Warning::FAIL_CLICK;
            observer.comment = "click Return(Enter)";
            return;
        }
    }
}

void InputService::click(ErrorList* result, int count, int delay)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.Screenshot();
    cv::Mat before, after;
    m_images.getMatObject().copyTo(before);
    cv::Rect clickRect = m_state.rect;

    focusWindow();
    PostMessage(m_state.game, WM_MOUSEMOVE, 0,
                MAKELPARAM(clickRect.x + 5, clickRect.y + 5));
    QThread::msleep(30);

    int x = 0;
    while (true) {
        PostMessage(m_state.game, WM_LBUTTONDOWN, MK_LBUTTON,
                    MAKELPARAM(clickRect.x + 5, clickRect.y + 5));
        QThread::msleep(delay);
        PostMessage(m_state.game, WM_LBUTTONUP, 0,
                    MAKELPARAM(clickRect.x + 5, clickRect.y + 5));
        QThread::msleep(15);
        SendMessage(m_state.game, WM_SETCURSOR, (WPARAM)m_state.game,
                    MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
        QThread::msleep(100);

        m_images.Screenshot();
        m_images.getMatObject().copyTo(after);
        m_images.compareObject(0, &after, &before, &l_result);

        if (!l_result)
            return;
        if (++x > count) {
            observer.value.warning = m_Warning::FAIL_CLICK;
            observer.comment = ("click failed at: " + QString::number(clickRect.x) + ";" + QString::number(clickRect.y) + "]");
            return;
        }
    }
}

void InputService::clickPosition(const cv::Rect& point, ErrorList* result, int count, int delay)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.Screenshot();
    cv::Mat before, after;
    m_images.getMatObject().copyTo(before);

    focusWindow();
    PostMessage(m_state.game, WM_MOUSEMOVE, 0,
                MAKELPARAM(point.x + 5, point.y + 5));
    QThread::msleep(30);

    int x = 0;
    while (true) {
        PostMessage(m_state.game, WM_LBUTTONDOWN, MK_LBUTTON,
                    MAKELPARAM(point.x + 5, point.y + 5));
        QThread::msleep(delay);
        PostMessage(m_state.game, WM_LBUTTONUP, 0,
                    MAKELPARAM(point.x + 5, point.y + 5));
        QThread::msleep(15);
        SendMessage(m_state.game, WM_SETCURSOR, (WPARAM)m_state.game,
                    MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
        QThread::msleep(100);

        m_images.Screenshot();
        m_images.getMatObject().copyTo(after);
        m_images.compareObject(0, &after, &before, &l_result);

        if (!l_result)
            return;
        if (++x > count) {
            observer.value.warning = m_Warning::FAIL_CLICK;
            observer.comment = ("click failed at: " + QString::number(point.x) + ";" + QString::number(point.y) + "]");
            return;
        }
    }
}

void InputService::clickSwipe(const cv::Rect& start, const cv::Rect& finish, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};
    m_images.Screenshot();
    cv::Mat before, after;
    m_images.getMatObject().copyTo(before);

    auto l_start = cv::Vec2i(start.x, start.y);
    auto l_stop  = cv::Vec2i(finish.x, finish.y);
    auto path    = l_stop - l_start;
    cv::Vec2f path_dir = path;
    path_dir = normalize(path_dir);

    SendMessage(m_state.main, WM_SETFOCUS, 0, 0);
    BYTE arr[256];
    memset(arr, 0, sizeof(256));
    GetKeyboardState(arr);
    auto oldBtn = arr[VK_LBUTTON];
    arr[VK_LBUTTON] = 128;
    SetKeyboardState(arr);

    SendMessage(m_state.main, WM_MOUSEACTIVATE, (WPARAM)m_state.main,
                MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    SendMessage(m_state.game, WM_SETCURSOR, (WPARAM)m_state.game,
                MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    PostMessage(m_state.game, WM_LBUTTONDOWN, MK_LBUTTON,
                MAKELPARAM(l_start[0], l_start[1]));
    QThread::msleep(15);

    const int steps = int(norm(path));
    cv::Vec2i last_pos;
    for (int i = 0; i < steps; ++i) {
        auto pos = cv::Vec2i(path_dir * float(i));
        last_pos = l_start + pos;
        SetKeyboardState(arr);
        PostMessage(m_state.game, WM_MOUSEMOVE, MK_LBUTTON,
                    MAKELPARAM(last_pos[0], last_pos[1]));

        const int pause = 50, offset = 15;
        if (i < offset)
            QThread::msleep(10 + ((offset - (i + 1)) * pause) / offset);
        else if (i > steps - offset)
            QThread::msleep(30 + ((offset - (steps - i)) * pause) / offset);
        else if (i % 5 == 0)
            QThread::msleep(1);
    }

    PostMessage(m_state.game, WM_LBUTTONUP, 0,
                MAKELPARAM(last_pos[0], last_pos[1]));
    QThread::msleep(15);
    SendMessage(m_state.game, WM_SETCURSOR, (WPARAM)m_state.game,
                MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
    QThread::msleep(15);

    arr[VK_LBUTTON] = oldBtn;
    SetKeyboardState(arr);

    m_images.Screenshot();
    m_images.getMatObject().copyTo(after);
    m_images.compareObject(0, &after, &before, &l_result);

    if (!l_result)
        return;

    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = ("swipe start: " + QString::number(start.x) + ";" + QString::number(start.y) + "] swipe finish: " + QString::number(finish.x) + ";" + QString::number(finish.y) + "]");
    return;
}
