#include "Controller/Controller.h"
#include "CustomError/ErrorObserver.h"
#include "ImageLibrary/ImageLibrary.h"

#include <QCoreApplication>
#include <QDir>
#include <QImage>

void Controller::saveImage(const QString &savePath, const cv::Mat &saveImage, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    if(!imwrite(savePath.toStdString(),saveImage)) {
        observer.value.error = m_Error::WRONG_IMG_PATH;
        observer.comment = ("save image, path: " + savePath);
        return;
    }
}

void Controller::Screenshot() {
    QCoreApplication::processEvents();
    int width = 900;
    int height = 600;
    HDC hdcWindow = GetDC(m_game);
    cv::Mat res(height, width, CV_8UC4);
    RECT rc;
    GetClientRect(m_game, &rc);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP bitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
    HGDIOBJ OBJ = SelectObject(hdcMem, bitmap);
    SelectObject(hdcMem, bitmap);
    BitBlt(hdcMem, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcWindow, 0, 0, SRCCOPY);
    SelectObject(hdcMem, OBJ);
    HDC hdcRar = GetDC(NULL);
    SelectObject(hdcRar, bitmap);
    StretchBlt(hdcRar, 0, 0, width, height, hdcMem, 0, 0, width, height, SRCCOPY);
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), width, height, 1, 32 };
    GetDIBits(hdcMem, bitmap, 0, height, res.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    DeleteObject(bitmap);
    DeleteDC(hdcMem);
    DeleteDC(hdcRar);
    ReleaseDC(m_game, hdcWindow);
    flip(res, m_object, 0);
}

cv::Mat Controller::getMatObject() { return m_object; }

void Controller::setMatObject(const cv::Mat &image, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_object.release();
    image.copyTo(m_object);

    if(m_object.empty()){
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMatObject->cv::Mat";
        return;
    }
}

void Controller::setMatObject(const QString &image, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    bool hasImage = false;
    lib->has(mainPath + "/" + image + ".png",hasImage);
    if(!hasImage) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMatObject->path: " + image;
        return;
    }
    else {
        m_object.release();
        lib->get(mainPath + "/" + image + ".png", m_object);
    }
    //QString path = QDir::cleanPath(mainPath + "/" + sample + ".png");
    // QImage image(path);
    // if(image.isNull()){
    //     observer.value.error = m_Error::WRONG_IMG_PATH;
    //     observer.comment = "setSample->path: " + path;
    //     return;
    // }
    // convertImage(image, &m_sample,result);
}

void Controller::setSample(const cv::Mat &sample, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_sample.release();
    sample.copyTo(m_sample);

    if(m_sample.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setSample->cv::Mat";
        return;
    }
}

void Controller::setSample(const QString &sample, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    bool hasImage = false;
    lib->has(mainPath + "/" + sample + ".png",hasImage);
    if(!hasImage){
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setSample->path: " + sample;
        return;
    }
    else {
        m_sample.release();
        lib->get(mainPath + "/" + sample + ".png", m_sample);
    }
    //QString path = QDir::cleanPath(mainPath + "/" + sample + ".png");
    // QImage image(path);
    // if(image.isNull()){
    //     observer.value.error = m_Error::WRONG_IMG_PATH;
    //     observer.comment = "setSample->path: " + path;
    //     return;
    // }
    // convertImage(image, &m_sample,result);
}

void Controller::setMask(const cv::Mat &mask, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    m_mask.release();
    mask.copyTo(m_mask);
    if(m_mask.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMask->cv::Mat";
        return;
    }
}

void Controller::setMask(const QString &mask, ErrorList *result) {
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, this, &Controller::LocalLogging);

    bool hasImage = false;
    lib->has(mainPath + "/" + mask + ".png",hasImage);
    if(!hasImage) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMask->path: " + (mainPath + "/" + mask + ".png");
        return;
    }
    else {
        m_mask.release();
        lib->get(mainPath + "/" + mask + ".png", m_mask);
    }

    // QString path = QDir::cleanPath(mainPath + "/" + mask + ".png");
    // QImage image(path);
    // if(image.isNull()){
    //     observer.value.error = m_Error::WRONG_IMG_PATH;
    //     observer.comment = "setMask->path: " + path;
    //     return;
    // }
    // convertImage(image, &m_mask, result);
}

void Controller::getImageFromLibrary(const QString &path, cv::Mat &result) {
    bool hasImage = false;
    lib->has(mainPath + "/" + path + ".png",hasImage);
    if(hasImage) {
        result.release();
        lib->get(mainPath + "/" + path + ".png", result);
    }
}
