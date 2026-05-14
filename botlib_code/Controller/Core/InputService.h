#pragma once

#include "Controller/ControllerState.h"
#include "Controller/Core/ImageService.h"

#include <functional>
#include <opencv2/opencv.hpp>
#include <QString>
#include <QObject>

struct ErrorList;

// Single responsibility: send mouse and keyboard WinAPI messages to the emulator window.
// Depends on ImageService only to take before/after screenshots for click-verification loops.
class InputService : public QObject
{
    Q_OBJECT
public:
    using LogFn = std::function<void(const QString&)>;

    InputService(ControllerState& state, ImageService& images, LogFn log);

    void clickEsc(ErrorList* result = nullptr, int count = 1);
    void clickReturn(ErrorList* result = nullptr, int count = 2);

    void click(ErrorList* result = nullptr, int count = 10, int delay = 100);
    void clickPosition(const cv::Rect& point, ErrorList* result = nullptr,
                       int count = 10, int delay = 100);
    void clickSwipe(const cv::Rect& start, const cv::Rect& finish,
                    ErrorList* result = nullptr);

private:
    void focusWindow();

    ControllerState& m_state;
    ImageService& m_images;
    LogFn m_log;
};
