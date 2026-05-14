#pragma once

#include "Controller/ControllerState.h"

#include <functional>
#include <opencv2/opencv.hpp>
#include <QString>
#include <QObject>

struct ErrorList;

// Single responsibility: all image I/O, processing, and template-matching operations.
// No game logic. No input handling.
class ImageService : public QObject
{
    Q_OBJECT
public:
    using LogFn = std::function<void(const QString&)>;

    explicit ImageService(ControllerState& state, LogFn log);

    // --- Capture ---
    void Screenshot();
    void saveImage(const QString& savePath, const cv::Mat& image, ErrorList* result = nullptr);

    // --- Accessors / mutators ---
    cv::Mat getMatObject();
    cv::Rect getRect();
    QString getMainPath();
    void getImageFromLibrary(const QString& path, cv::Mat& result);

    void setMatObject(const cv::Mat& image, ErrorList* result = nullptr);
    void setMatObject(const QString& image, ErrorList* result = nullptr);
    void setSample(const cv::Mat& sample, ErrorList* result = nullptr);
    void setSample(const QString& sample, ErrorList* result = nullptr);
    void setMask(const cv::Mat& mask, ErrorList* result = nullptr);
    void setMask(const QString& mask, ErrorList* result = nullptr);

    // --- Processing ---
    cv::Mat cutImage();
    void changeColor(const cv::Mat& before, cv::Mat* after,
                     ErrorList* result = nullptr, bool convert = false);

    // --- Detection ---
    void findObject(const cv::Mat* finder = nullptr, ErrorList* result = nullptr);
    void compareObject(double rightVal = 0.02, const cv::Mat* object = nullptr,
                       const cv::Mat* sample = nullptr, ErrorList* result = nullptr);
    void compareSample(const QString& pagePath, const QString& samplePath,
                       const QString& maskPath, ErrorList* result = nullptr,
                       bool shoot = false, double rightVal = 0.02);

private:
    ControllerState& m_state;
    LogFn m_log;
};
