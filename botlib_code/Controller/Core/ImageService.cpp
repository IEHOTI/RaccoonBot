#include "Controller/Core/ImageService.h"
#include "CustomError/ErrorObserver.h"
#include "ImageLibrary/ImageLibrary.h"

#include <QCoreApplication>
#include <QDebug>

ImageService::ImageService(ControllerState& state, LogFn log)
    : m_state(state)
    , m_log(std::move(log))
{
}

// ---- Capture ----------------------------------------------------------------

void ImageService::Screenshot()
{
    QCoreApplication::processEvents();
    const int width = 900;
    const int height = 600;

    HDC hdcWindow = GetDC(m_state.game);
    cv::Mat res(height, width, CV_8UC4);

    RECT rc;
    GetClientRect(m_state.game, &rc);

    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP bitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
    HGDIOBJ obj = SelectObject(hdcMem, bitmap);
    SelectObject(hdcMem, bitmap);
    BitBlt(hdcMem, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcWindow, 0, 0, SRCCOPY);
    SelectObject(hdcMem, obj);

    HDC hdcRar = GetDC(NULL);
    SelectObject(hdcRar, bitmap);
    StretchBlt(hdcRar, 0, 0, width, height, hdcMem, 0, 0, width, height, SRCCOPY);

    BITMAPINFOHEADER bi = {sizeof(BITMAPINFOHEADER), width, height, 1, 32};
    GetDIBits(hdcMem, bitmap, 0, height, res.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(bitmap);
    DeleteDC(hdcMem);
    DeleteDC(hdcRar);
    ReleaseDC(m_state.game, hdcWindow);

    flip(res, m_state.object, 0);
}

void ImageService::saveImage(const QString& savePath, const cv::Mat& image, ErrorList* result)
{
    QCoreApplication::processEvents();
    qDebug() << "try to save in: " << savePath;
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    if (!imwrite(savePath.toStdString(), image)) {
        observer.value.error = m_Error::WRONG_IMG_PATH;
        observer.comment = "save image, path: " + savePath;
        return;
    }
}

// ---- Accessors / mutators ---------------------------------------------------

cv::Mat ImageService::getMatObject()
{
    QCoreApplication::processEvents();
    return m_state.object;
}

cv::Rect ImageService::getRect()
{
    return m_state.rect;
}

QString ImageService::getMainPath()
{
    return m_state.mainPath;
}

void ImageService::getImageFromLibrary(const QString& path, cv::Mat& result)
{
    QCoreApplication::processEvents();
    bool hasImage = false;
    m_state.lib->has(m_state.mainPath + "/" + path + ".png", hasImage);
    if (hasImage) {
        result.release();
        m_state.lib->get(m_state.mainPath + "/" + path + ".png", result);
    }
}

void ImageService::setMatObject(const cv::Mat& image, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    m_state.object.release();
    image.copyTo(m_state.object);

    if (m_state.object.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMatObject->cv::Mat";
        return;
    }
}

void ImageService::setMatObject(const QString& image, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    bool hasImage = false;
    m_state.lib->has(m_state.mainPath + "/" + image + ".png", hasImage);
    if (!hasImage) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMatObject->path: " + image;
        return;
    }
    m_state.object.release();
    m_state.lib->get(m_state.mainPath + "/" + image + ".png", m_state.object);
}

void ImageService::setSample(const cv::Mat& sample, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    m_state.sample.release();
    sample.copyTo(m_state.sample);

    if (m_state.sample.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setSample->cv::Mat";
        return;
    }
}

void ImageService::setSample(const QString& sample, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    bool hasImage = false;
    m_state.lib->has(m_state.mainPath + "/" + sample + ".png", hasImage);
    if (!hasImage) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setSample->path: " + sample;
        return;
    }
    m_state.sample.release();
    m_state.lib->get(m_state.mainPath + "/" + sample + ".png", m_state.sample);
}

void ImageService::setMask(const cv::Mat& mask, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    m_state.mask.release();
    mask.copyTo(m_state.mask);

    if (m_state.mask.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMask->cv::Mat";
        return;
    }
}

void ImageService::setMask(const QString& mask, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    bool hasImage = false;
    m_state.lib->has(m_state.mainPath + "/" + mask + ".png", hasImage);
    if (!hasImage) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "setMask->path: " + (m_state.mainPath + "/" + mask + ".png");
        return;
    }
    m_state.mask.release();
    m_state.lib->get(m_state.mainPath + "/" + mask + ".png", m_state.mask);
}

// ---- Processing -------------------------------------------------------------

cv::Mat ImageService::cutImage()
{
    return m_state.object(m_state.rect);
}

void ImageService::changeColor(const cv::Mat& before, cv::Mat* after, ErrorList* result,
                               bool convert)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    if (before.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "before";
        return;
    }

    cv::Mat temp = before.clone();
    if (temp.channels() == 4)
        cvtColor(temp, temp, cv::COLOR_RGBA2RGB);
    if (convert)
        cv::cvtColor(temp, temp, cv::COLOR_RGB2BGR);

    cv::Scalar lowerWhite(200, 200, 200), upperWhite(255, 255, 255);
    cv::Scalar lowerBlackBlue(0, 0, 0),   upperBlackBlue(100, 100, 100);
    cv::Scalar lowerBlue(100, 150, 0),    upperBlue(255, 255, 100);
    cv::Scalar lowerNumberBlue(105, 95, 15),  upperNumberBlue(165, 160, 130);
    cv::Scalar lowerYellow(15, 95, 105),  upperYellow(75, 135, 140);

    cv::Scalar dullGreen(144, 238, 144);
    cv::Scalar white(255, 255, 255);

    cv::Mat mask;
    cv::inRange(temp, lowerWhite, upperWhite, mask);
    temp.setTo(dullGreen, mask);

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

// ---- Detection --------------------------------------------------------------

void ImageService::findObject(const cv::Mat* finder, ErrorList* result)
{
    QCoreApplication::processEvents();
    if (finder != nullptr)
        finder->copyTo(m_state.mask);

    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    if (m_state.mask.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "findObject->mask: ";
        return;
    }

    cv::Mat grayImage;
    cvtColor(m_state.mask, grayImage, cv::COLOR_BGR2GRAY);

    cv::Mat binaryImage;
    threshold(grayImage, binaryImage, 127, 255, cv::THRESH_BINARY_INV);

    std::vector<std::vector<cv::Point>> contours;
    findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    m_state.rect = boundingRect(contours[0]);
    if (m_state.rect.width == 0 || m_state.rect.height == 0) {
        observer.value.warning = m_Warning::FAIL_CHECK;
        observer.comment = "empty area of find";
        return;
    }
}

void ImageService::compareObject(double rightVal, const cv::Mat* object,
                                 const cv::Mat* sample, ErrorList* result)
{
    QCoreApplication::processEvents();
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    cv::Mat img1, sample1;

    if (object != nullptr)
        object->copyTo(img1);
    else
        m_state.object.copyTo(img1);

    if (img1.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "compareObject->object && m_object";
        return;
    }

    if (sample != nullptr)
        sample->copyTo(sample1);
    else
        m_state.sample.copyTo(sample1);

    if (sample1.empty()) {
        observer.value.error = m_Error::EMPTY_IMG;
        observer.comment = "compareObject->sample && m_sample";
        return;
    }

    m_state.rect.x = 0;
    m_state.rect.y = 0;
    cvtColor(img1, img1, cv::COLOR_BGR2GRAY);
    cvtColor(sample1, sample1, cv::COLOR_BGR2GRAY);

    int res_col = sample1.cols - img1.cols + 1;
    int res_row = sample1.rows - img1.rows + 1;
    cv::Mat resultMat(res_row, res_col, CV_32FC1);
    cv::matchTemplate(img1, sample1, resultMat, cv::TM_SQDIFF_NORMED);

    double minVal = 0;
    cv::Point minLoc;
    minMaxLoc(resultMat, &minVal, nullptr, &minLoc, nullptr);
    m_log("==MatchTemplate: Result[" + QString::number(minVal) + "]; RightVal["
          + QString::number(rightVal) + "]");

    if (minVal <= rightVal) {
        m_state.rect.x = minLoc.x;
        m_state.rect.y = minLoc.y;
        return;
    }

    observer.value.warning = m_Warning::FAIL_COMPARE;
    observer.print = false;
    return;
}

void ImageService::compareSample(const QString& pagePath, const QString& samplePath,
                                 const QString& maskPath, ErrorList* result,
                                 bool shoot, double rightVal)
{
    ErrorObserver observer(result);
    connect(&observer, &ErrorObserver::Logging, [this](const QString& msg) { m_log(msg); });

    if (shoot)
        Screenshot();

    if (m_state.object.empty()) {
        observer.value.error = m_Error::NO_ACTIVE_EMULATOR;
        return;
    }

    ErrorList l_result = {m_Warning::NO_WARN, m_Error::NO_ERR};

    m_log("Converting mask: " + (pagePath + "/" + maskPath));
    setMask(pagePath + "/" + maskPath, &l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.comment = "compareSample->convertImage->mask: "
                + (m_state.mainPath + "/" + pagePath + "/" + maskPath + ".png");
        return;
    }

    findObject(nullptr, &l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.print = false;
        return;
    }

    m_log("Converting sample: " + (pagePath + "/" + samplePath));
    setSample(pagePath + "/" + samplePath, &l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.comment = "compareSample->convertImage->sample: "
                + (m_state.mainPath + "/" + pagePath + "/" + samplePath + ".png");
        return;
    }

    cv::Mat img1 = m_state.sample(m_state.rect);
    compareObject(rightVal, &img1, &m_state.object, &l_result);
    if (!l_result) {
        observer.value = l_result;
        observer.print = false;
    }
}
