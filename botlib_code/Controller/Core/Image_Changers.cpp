#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QCoreApplication>
#include <QImage>

// void Controller::convertImage(const QImage &imageOne, cv::Mat *imageTwo, ErrorList *result) {
//     QCoreApplication::processEvents();
//     ErrorObserver observer(result);
//     connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
//     // Проверка входных данных
//     if (imageOne.isNull()) {
//         observer.value.error = m_Error::WRONG_IMG_PATH;
//         observer.print = false;
//         return;
//     }
//     // Проверка указателя
//     if (!imageTwo) {
//         observer.value.error = m_Error::EMPTY_IMG;
//         observer.print = false;
//         return;
//     }
//     // Определяем формат QImage и конвертируем в соответствующий cv::Mat
//     cv::Mat cvImg;
//     switch (imageOne.format()) {
//     case QImage::Format_RGB32:
//     case QImage::Format_ARGB32:
//         cvImg = cv::Mat(imageOne.height(), imageOne.width(), CV_8UC4, (void*)imageOne.constBits(), imageOne.bytesPerLine());
//         cvtColor(cvImg, cvImg, cv::COLOR_RGBA2RGB);
//         break;
//     case QImage::Format_Grayscale8:
//         cvImg = cv::Mat(imageOne.height(), imageOne.width(), CV_8UC1, (void*)imageOne.bits(), imageOne.bytesPerLine());
//         break;
//     default:
//         observer.value.warning = m_Warning::UNKNOWN;
//         observer.comment = "unsupported format";
//         return;
//     }
//     imageTwo->release();
//     // Копируем результат
//     cvImg.copyTo(*imageTwo);
// }

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

    //imwrite("C:/Utilities/Coding/Photo/ocr/controller_change_color_before.png",before);

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

    //imwrite("C:/Utilities/Coding/Photo/ocr/controller_change_color_after.png",*after);

    if (after->empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "after";
        return;
    }
}
