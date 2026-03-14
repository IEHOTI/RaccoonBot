#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QCoreApplication>
#include <QImage>

cv::Mat Controller::cutImage() { return m_object(m_rect); }

void Controller::changeColor(const cv::Mat &before, cv::Mat *after, ErrorList *result, bool convert) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    if (before.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "before";
        return;
    }

    // Делаем копию, чтобы не трогать оригинал
    cv::Mat temp = before.clone();
    if(temp.channels() == 4) cvtColor(temp, temp, cv::COLOR_RGBA2RGB);
    if(convert) cv::cvtColor(temp, temp, cv::COLOR_RGB2BGR);
    // Диапазоны (BGR!)
    cv::Scalar lowerWhite(200, 200, 200);
    cv::Scalar upperWhite(255, 255, 255);

    cv::Scalar lowerBlackBlue(0, 0, 0);
    cv::Scalar upperBlackBlue(100, 100, 100);

    cv::Scalar lowerBlue(100, 150, 0);
    cv::Scalar upperBlue(255, 255, 100);

    cv::Scalar lowerNumberBlue(105, 95, 15);
    cv::Scalar upperNumberBlue(165, 160, 130);

    cv::Scalar lowerYellow(15,95,105);
    cv::Scalar upperYellow(75,135,140);

    // Цвета замены (BGR!)
    cv::Scalar dullGreen(144, 238, 144);
    cv::Scalar white(255, 255, 255);

    cv::Mat mask;

    // Белое → зеленое
    cv::inRange(temp, lowerWhite, upperWhite, mask);
    temp.setTo(dullGreen, mask);

    // * → белое
    cv::Mat mask1, mask2, mask3, mask4;
    cv::inRange(temp, lowerBlackBlue, upperBlackBlue, mask1);
    cv::inRange(temp, lowerBlue, upperBlue, mask2);
    cv::inRange(temp, lowerNumberBlue, upperNumberBlue, mask3);
    cv::inRange(temp, lowerYellow, upperYellow, mask4);

    cv::bitwise_or(mask1, mask2, mask);

    cv::bitwise_or(mask, mask3, mask);
    cv::bitwise_or(mask, mask4, mask);

    temp.setTo(white, mask);
    temp.copyTo(*after);

    if (after->empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "after";
        return;
    }
}
