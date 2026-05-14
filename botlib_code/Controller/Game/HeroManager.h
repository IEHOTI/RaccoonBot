#pragma once

#include "Controller/Core/ImageService.h"
#include "Controller/Core/InputService.h"
#include "Controller/Game/GameNavigator.h"
#include "Task/TaskSettings.h"

#include <functional>
#include <QList>
#include <QString>
#include <QObject>

struct ErrorList;

// Single responsibility: manage hero page navigation and hero equipment operations.
class HeroManager : public QObject
{
    Q_OBJECT
public:
    using LogFn     = std::function<void(const QString&)>;
    using LogMainFn = std::function<void(const QString&, bool)>;
    using FixFn     = std::function<void()>;

    HeroManager(ImageService& images, InputService& input,
                GameNavigator& navigator, LogFn log, LogMainFn logMain, FixFn fix);

    void openHeroPage(ErrorList* result = nullptr);
    void setHeroSet(typeSet set = typeSet::NOT_TOUCH, ErrorList* result = nullptr);
    void setHeroRelics(ErrorList* result = nullptr,
                       const QList<QString>* relicsList = nullptr);

private:
    ImageService& m_images;
    InputService& m_input;
    GameNavigator& m_nav;
    LogFn m_log;
    LogMainFn m_logMain;
    FixFn m_fix;
};
