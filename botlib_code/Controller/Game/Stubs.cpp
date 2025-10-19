#include "Controller/Controller.h"

void Controller::getGameError() {
    if(m_object.empty()) emit errorLogging("Пустой m_object");
    else imshow("1",m_object);
    if(m_mask.empty()) emit errorLogging("Пустой m_mask");
    else imshow("2",m_mask);
    if(m_sample.empty()) emit errorLogging("Пустой m_sample");
    else imshow("3",m_sample);
}
