#pragma once

#include "ImageLibrary/ImageLibrary.h"

#include <opencv2/opencv.hpp>
#include <windows.h>

#include <QString>

// Shared data context — plain holder, no behaviour.
// Passed by reference into each service so they operate on the same state.
struct ControllerState
{
    HWND main = 0;
    HWND game = 0;

    cv::Mat object, sample, mask;
    cv::Rect rect{0, 0, 0, 0};

    QString mainPath = ":/pages";
    ImageLibrary* lib = nullptr;

    bool isWorking = false;
};
