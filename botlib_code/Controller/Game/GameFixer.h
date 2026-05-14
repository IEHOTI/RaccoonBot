#pragma once

#include "Controller/Core/ImageService.h"
#include "Controller/Core/InputService.h"
#include "Controller/Game/GameNavigator.h"
#include "Controller/Game/GameStateManager.h"

#include <functional>
#include <QString>
#include <QObject>

struct ErrorList;

// Single responsibility: recover from game errors, pop-ups, and inconsistent states.
// Depends on GameStateManager for checks, but GameStateManager does NOT depend on GameFixer,
// so there is no circular dependency.
class GameFixer : public QObject
{
    Q_OBJECT
public:
    using LogFn      = std::function<void(const QString&)>;
    using LogMainFn  = std::function<void(const QString&, bool)>;

    GameFixer(ImageService& images, InputService& input,
              GameNavigator& navigator, GameStateManager& checker,
              LogFn log, LogMainFn logMain);

    void fixGameError(ErrorList* result = nullptr);
    void fixPopUpError(ErrorList* result = nullptr);
    void fixBattleSettings();
    void refreshMainPage(ErrorList* result = nullptr);

private:
    ImageService& m_images;
    InputService& m_input;
    GameNavigator& m_nav;
    GameStateManager& m_checker;
    LogFn m_log;
    LogMainFn m_logMain;
};
