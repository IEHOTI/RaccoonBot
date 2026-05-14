#pragma once

#include "Controller/Core/ImageService.h"
#include "Controller/Core/InputService.h"

#include <functional>
#include <QString>
#include <QObject>

struct ErrorList;

// Single responsibility: high-level navigation actions —
// clicking named buttons and swiping the map to locate buttons.
// Also owns checkMap since map-presence is a navigation precondition.
class GameNavigator : public QObject
{
    Q_OBJECT
public:
    using LogFn = std::function<void(const QString&)>;
    using FixFn = std::function<void()>;

    GameNavigator(ImageService& images, InputService& input, LogFn log, FixFn fix);

    void clickButton(const QString& pagePath, const QString& buttonName,
                     ErrorList* result = nullptr, int count = 3, int delay = 100);
    void clickMapButton(const QString& pageName, const QString& buttonName,
                        ErrorList* result = nullptr, int count = 3, int delay = 100);

    void openAnySets(ErrorList* result = nullptr);

    // Map-presence check lives here because it is a navigation precondition used by clickMapButton.
    void checkMap(ErrorList* result = nullptr);

private:
    ImageService& m_images;
    InputService& m_input;
    LogFn m_log;
    FixFn m_fix;
};
