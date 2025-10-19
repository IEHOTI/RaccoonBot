#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"

#include <QCoreApplication>
#include <QImage>


void Controller::findObject(const cv::Mat *finder, ErrorList *result) {
    QCoreApplication::processEvents();
    if(finder != nullptr) finder->copyTo(m_mask);
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);
    if (m_mask.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "findObject->mask: ";
        return;
    }
    // Преобразуем изображение в оттенки серого
    cv::Mat grayImage;
    cvtColor(m_mask, grayImage, cv::COLOR_BGR2GRAY);
    // Бинаризуем изображение
    cv::Mat binaryImage;
    threshold(grayImage, binaryImage, 127, 255, cv::THRESH_BINARY_INV);
    // Находим контуры
    std::vector<std::vector<cv::Point>> contours;
    findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    // Находим прямоугольник, описывающие контур
    m_rect = boundingRect(contours[0]);
    if (m_rect.width == 0 || m_rect.height == 0) {
        observer.value.warning = m_Warning::FAIL_CHECK;
        observer.comment = "empty area of find";
        return;
    }
}

void Controller::compareObject(double rightVal, const cv::Mat *object, const cv::Mat *sample, ErrorList *result) {
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    cv::Mat img1, sample1;

    if (object != nullptr) object->copyTo(img1);
    else m_object.copyTo(img1);
    if (img1.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "compareObject->object && m_object";
        return;
    }
    if (sample != nullptr) sample->copyTo(sample1);
    else m_sample.copyTo(sample1);
    if (sample1.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "compareObject->sample && m_sample";
        return;
    }

    m_rect.x = 0;
    m_rect.y = 0;
    cvtColor(img1, img1, cv::COLOR_BGR2GRAY);
    cvtColor(sample1, sample1, cv::COLOR_BGR2GRAY);

    // Создание матрицы результата
    int res_col = sample1.cols - img1.cols + 1;
    int res_row = sample1.rows - img1.rows + 1;
    cv::Mat resultMat(res_row, res_col, CV_32FC1);

    // Сравнение шаблона с изображением
    cv::matchTemplate(img1, sample1, resultMat, cv::TM_SQDIFF_NORMED);

    // Нахождение наилучшего совпадения
    double minVal = 0;
    cv::Point minLoc;
    minMaxLoc(resultMat, &minVal, nullptr, &minLoc, nullptr);
    emit errorLogging("==MatchTemplate: Result[" + QString::number(minVal) + "]; RightVal[" + QString::number(rightVal) + "]");

    // Проверка наилучшего совпадения
    if (minVal <= rightVal) {
        m_rect.x = minLoc.x;
        m_rect.y = minLoc.y;
        return;
    }

    observer.value.warning = m_Warning::FAIL_COMPARE;
    observer.print = false;
    return;
}

void Controller::compareSample(const QString &pagePath, const QString &samplePath, const QString &maskPath, ErrorList *result, bool shoot, double rightVal) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    if(shoot) Screenshot();
    if (m_object.empty()) {
        observer.value.error = m_Error::NO_ACTIVE_EMULATOR;
        return;
    }

    ErrorList l_result = {m_Warning::NO_WARN,m_Error::NO_ERR};
    //Нахождение координат обрезаемой области(findObject)
    emit errorLogging("Converting mask: " + (pagePath + "/" + maskPath));

    // bool hasImage = false;
    // lib->has(mainPath + "/" + pagePath + "/" + maskPath + ".png",hasImage);
    // if(!hasImage) throw ImageException(QString("Hasnt image " + mainPath + "/" + pagePath + "/" + maskPath + ".png").toStdString());
    // else lib->get(mainPath + "/" + pagePath + "/" + maskPath + ".png", m_mask);
    // why not setMask???
    // convertImage(QImage((mainPath + "/" + pagePath + "/" + maskPath + ".png")), &m_mask, &l_result);

    setMask(pagePath + "/" + maskPath,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.comment = "compareSample->convertImage->mask: " + (mainPath + "/" + pagePath + "/" + maskPath + ".png");
        return;
    }

    findObject(nullptr,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
    emit errorLogging("Converting sample: " + (pagePath + "/" + samplePath));

    // lib->has(mainPath + "/" + pagePath + "/" + samplePath + ".png", hasImage);
    // if(!hasImage) throw ImageException(QString("Hasnt image " + mainPath + "/" + pagePath + "/" + samplePath + ".png").toStdString());
    // else lib->get(mainPath + "/" + pagePath + "/" + samplePath + ".png", m_sample);
    //convertImage(QImage((mainPath + "/" + pagePath + "/" + samplePath + ".png")), &m_sample,&l_result);

    setSample(pagePath + "/" + samplePath,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.comment = "compareSample->convertImage->sample: " + (mainPath + "/" + pagePath + "/" + samplePath + ".png");
        return;
    }

    cv::Mat img1 = m_sample(m_rect);
    compareObject(rightVal,&img1,&m_object,&l_result);
    if(!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }
}
