#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QImage>

void Controller::clickButton(const QString &pagePath, const QString &buttonName, ErrorList *result, int count, int delay) {
    emit errorLogging("=Клик по кнопке:\"" + mainPath + "/" + pagePath + "/" + buttonName + ".png" + "\"");
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};

    //m_mask.release();
    //convertImage(QImage((mainPath + "/" + pagePath + "/" + buttonName + ".png")), &m_mask,&l_result);
    setMask(pagePath + "/" + buttonName,&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.comment = "convert->mask";
        return;
    }
    findObject(nullptr,&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    click(result,count,delay);
}
void Controller::clickMapButton(const QString &mapName, const QString &buttonName, ErrorList *result, int count, int delay) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    checkMap(&l_result);
    if(!l_result){
        observer.value = l_result;
        observer.print = false;
        return;
    }
    int x = 0;
    cv::Rect start = {690,290,0,0};
    cv::Rect finish = {100,290,0,0};
    while (x < 3){//right swipe
        compareSample("map",mapName,buttonName,&l_result,true);
        if(!l_result) {
            clickSwipe(start,finish);
            x++;
        }
        else break;
    }
    if(!l_result) {//left swipe
        x = 0;
        while(x < 3){
            compareSample("map",mapName,buttonName,&l_result,true);
            if(!l_result) {
                clickSwipe(finish,start);
                x++;
            }
            else break;
        }
    }
    if(!l_result) {
        observer.value.warning = m_Warning::FAIL_PAGE;
        observer.comment = "Cant find " + buttonName;
        return;
    }
    else click(result,count,delay);
}
