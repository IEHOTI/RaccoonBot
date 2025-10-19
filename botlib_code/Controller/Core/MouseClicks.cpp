#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QCoreApplication>
#include <QThread>

void Controller::click(ErrorList *result, int count, int delay) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();
    cv::Mat before,after;
    m_object.copyTo(before);
    cv::Rect click = m_rect;
    int x = 0;
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    QThread::msleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    do {
        after.release();
        PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(click.x + 5, click.y + 5));
        QThread::msleep(delay);
        PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(click.x + 5, click.y + 5));
        QThread::msleep(100);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0, &after, &before, &l_result);
        if (!l_result) return;
        QThread::msleep(delay);
        x++;
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = ("bot pos: " + QString::number(m_rect.x) + ";" + QString::number(m_rect.y) + "]");
    return;
}

void Controller::clickPosition(const cv::Rect &point, ErrorList *result, int count, int delay) {
    QCoreApplication::processEvents();

    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    Screenshot();

    cv::Mat before,after;
    m_object.copyTo(before);

    int x = 0;
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    QThread::msleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    do {
        after.release();
        PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(point.x + 5, point.y + 5));
        QThread::msleep(delay);
        PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(point.x + 5, point.y + 5));
        QThread::msleep(100);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0, &after, &before, &l_result);
        if (!l_result) return;
        QThread::msleep(delay);
        x++;
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = ("user pos: " + QString::number(m_rect.x) + ";" + QString::number(m_rect.y) + "]");
    return;
}

void Controller::clickSwipe(const cv::Rect &start, const cv::Rect &finish, ErrorList *result) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();
    cv::Mat before,after;
    m_object.copyTo(before);

    auto l_start = cv::Vec2i(start.x, start.y);
    auto l_stop = cv::Vec2i(finish.x, finish.y);
    auto path = l_stop - l_start;
    cv::Vec2f path_dir = path;
    path_dir = normalize(path_dir);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    BYTE arr[256];
    memset(arr, 0, sizeof(256));
    GetKeyboardState(arr);
    auto old = arr[VK_LBUTTON];
    arr[VK_LBUTTON] = 128;
    SetKeyboardState(arr);

    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(l_start[0], l_start[1]));
    QThread::msleep(15);

    const int count = int(norm(path));
    cv::Vec2i last_pos;

    for (int i = 0; i < count; ++i) {
        auto pos = cv::Vec2i(path_dir  *float(i));
        last_pos = l_start + pos;

        SetKeyboardState(arr);
        PostMessage(m_game, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(last_pos[0], last_pos[1]));

        const int pause = 50;
        const int offset = 15;
        if (i < offset) {
            auto p = 10 + ((offset - (i + 1))  *pause) / offset;
            QThread::msleep(p);
        }
        else if (i > count - offset) {
            auto p = 30 + ((offset - (count - i))  *pause) / offset;
            QThread::msleep(p);
        }
        else if (i % 5 == 0) QThread::msleep(1);
    }

    PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(last_pos[0], last_pos[1]));
    QThread::msleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
    QThread::msleep(15);

    arr[VK_LBUTTON] = old;
    SetKeyboardState(arr);

    Screenshot();
    m_object.copyTo(after);
    compareObject(0, &after, &before, &l_result);
    if (!l_result) return;
    else {
        observer.value.warning = m_Warning::FAIL_CLICK;
        observer.comment = ("swipe start: " + QString::number(start.x) + ";" + QString::number(start.y) + "] swipe finish: " + QString::number(finish.x) + ";" + QString::number(finish.y) + "]");
        return;
    }

}
