#pragma once

#include "Controller/ControllerState.h"
#include "Controller/Core/ImageService.h"
#include "Controller/Core/InputService.h"
#include "Controller/Game/GameNavigator.h"

#include <functional>
#include <QObject>

struct ErrorList;

// Single responsibility: validate and wait for the correct game screen state.
// Covers emulator validity checks, loading screens, and page-presence verification.
// Does NOT fix errors — that is GameFixer's responsibility.
class GameStateManager : public QObject
{
    Q_OBJECT
public:
    using LogFn = std::function<void(const QString&)>;
    using FixFn = std::function<void()>;

    GameStateManager(ControllerState& state, ImageService& images, InputService& input,
                     GameNavigator& navigator, LogFn log, FixFn fix);

    // Emulator validity
    void isEmpty(ErrorList* result = nullptr);
    void isValidSize(ErrorList* result = nullptr);
    void isValidPos(ErrorList* result = nullptr);

    // Loading waits
    void checkLoading();
    void checkGameLoading();

    // Page checks
    void checkPreMainPage();
    void checkMainPage(ErrorList* result = nullptr);
    void checkEvent(ErrorList* result = nullptr);
    void checkSettings(ErrorList* result = nullptr);

    // Internal helpers also used by GameFixer
    void skipEvent();

private:
    ControllerState& m_state;
    ImageService& m_images;
    InputService& m_input;
    GameNavigator& m_nav;
    LogFn m_log;
    FixFn m_fix;
};
