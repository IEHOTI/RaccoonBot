#pragma once

#include "Controller/Core/ImageService.h"
#include "Controller/Game/GameNavigator.h"

#include <functional>
#include <QObject>

struct ErrorList;
struct userProfile;

// Single responsibility: scan and populate a userProfile from the in-game user screen.
class UserManager : public QObject
{
    Q_OBJECT
public:
    using LogFn       = std::function<void(const QString&)>;
    using RecognizeFn = std::function<void(const cv::Mat&, int&)>;
    using FixFn       = std::function<void()>;

    UserManager(ImageService& images, GameNavigator& navigator,
                LogFn log, RecognizeFn recognize, FixFn fix);

    void userInitialize(userProfile* user, ErrorList* result = nullptr);

private:
    ImageService& m_images;
    GameNavigator& m_nav;
    LogFn m_log;
    RecognizeFn m_recognize;
    FixFn m_fix;
};
