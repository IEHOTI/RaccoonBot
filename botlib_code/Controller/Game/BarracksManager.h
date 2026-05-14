#pragma once

#include "Controller/Core/ImageService.h"
#include "Controller/Core/InputService.h"
#include "Controller/Game/GameNavigator.h"
#include "Task/TaskSettings.h"

#include <functional>
#include <QString>
#include <QObject>

struct ErrorList;
struct userProfile;

// Single responsibility: manage barracks discovery, entry, squad scanning, and unit equipment.
class BarracksManager : public QObject
{
    Q_OBJECT
public:
    using LogFn        = std::function<void(const QString&)>;
    using LogMainFn    = std::function<void(const QString&, bool)>;
    using RecognizeFn  = std::function<void(const cv::Mat&, int&)>;
    using FixFn        = std::function<void()>;

    BarracksManager(ImageService& images, InputService& input,
                    GameNavigator& navigator,
                    LogFn log, LogMainFn logMain, RecognizeFn recognize, FixFn fix);

    void findBarracks(ErrorList* result = nullptr);
    void entryBarracks(ErrorList* result = nullptr);
    void scanSquadCount(userProfile* user, ErrorList* result = nullptr);
    void setUnitSet(int index, typeSet set = typeSet::NOT_TOUCH, ErrorList* result = nullptr);

private:
    ImageService& m_images;
    InputService& m_input;
    GameNavigator& m_nav;
    LogFn m_log;
    LogMainFn m_logMain;
    RecognizeFn m_recognize;
    FixFn m_fix;
};
