#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QThread>

void Controller::clickEsc(ErrorList *result, int count) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();
    cv::Mat before,after;
    m_object.copyTo(before);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    QThread::msleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    int x = 0;
    do{
        after.release();
        PostMessage(m_game, WM_KEYDOWN, VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
        QThread::msleep(150);
        PostMessage(m_game, WM_KEYUP, VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
        QThread::msleep(100);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0, &after,&before,&l_result);
        if(!l_result) return;
        else x++;
        QThread::msleep(150);
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = "click Esc";
    return;
}

void Controller::clickReturn(ErrorList *result, int count) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    Screenshot();
    cv::Mat before,after;
    m_object.copyTo(before);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    QThread::msleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    QThread::msleep(15);
    int x = 0;
    do{
        PostMessage(m_game, WM_KEYDOWN, VK_RETURN, 0);
        QThread::msleep(150);
        PostMessage(m_game, WM_KEYUP, VK_RETURN, 0);
        QThread::msleep(100);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0, &after,&before,&l_result);
        if(!l_result) return;
        else x++;
        QThread::msleep(150);
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    observer.value.warning = m_Warning::FAIL_CLICK;
    observer.comment = "click Return(Enter)";
    return;
}
