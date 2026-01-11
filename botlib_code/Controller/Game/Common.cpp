#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include <QThread>

void Controller::openAnySets(ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    QString pagePath = "squad/main/unit";
    int x = 0;
    do{
        compareSample(pagePath,"sample_set","compare_set",&l_result,true);
        if(!l_result) {
            compareSample(pagePath,"sample","button_set",&l_result);
            if(!l_result) {
                x++;
                QThread::msleep(1000);
                if(x == 10){
                    observer.value.warning = m_Warning::FAIL_COMPARE;
                    observer.comment = "setSomeSets->buttonSet";
                    return;
                }
            }
            else click(&l_result,1);
        }
    } while(!l_result);
}
