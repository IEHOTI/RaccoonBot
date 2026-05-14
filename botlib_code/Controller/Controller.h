#pragma once

#include "Controller/ControllerState.h"
#include "Controller/Core/ImageService.h"
#include "Controller/Core/InputService.h"
#include "Controller/Game/BarracksManager.h"
#include "Controller/Game/GameFixer.h"
#include "Controller/Game/GameNavigator.h"
#include "Controller/Game/GameStateManager.h"
#include "Controller/Game/HeroManager.h"
#include "Controller/Game/UserManager.h"
#include "Task/TaskSettings.h"

#include <QObject>
#include <opencv2/opencv.hpp>
#include <windows.h>

class Emulator;
struct ErrorList;
struct userProfile;

// Thin orchestrator and Qt signal hub.
// Owns all service objects and the shared ControllerState.
// Contains no game/image logic itself — delegates everything to the appropriate service.
class BOTLIB_EXPORT Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject* parent = nullptr);
    ~Controller();
    void CleanUp();

    // Core
    void imageLoader(ImageLibrary *lib);

    // --- Delegated: ImageService ---
    cv::Mat cutImage();
    void changeColor(const cv::Mat& before, cv::Mat* after,
                     ErrorList* result = nullptr, bool convert = false);
    void findObject(const cv::Mat* finder = nullptr, ErrorList* result = nullptr);
    void compareObject(double rightVal = 0.02, const cv::Mat* object = nullptr,
                       const cv::Mat* sample = nullptr, ErrorList* result = nullptr);
    void compareSample(const QString& pagePath, const QString& samplePath,
                       const QString& maskPath, ErrorList* result = nullptr,
                       bool shoot = false, double rightVal = 0.02);
    void Screenshot();
    void saveImage(const QString& savePath, const cv::Mat& image, ErrorList* result = nullptr);
    void setMatObject(const QString& image, ErrorList* result = nullptr);
    void setMatObject(const cv::Mat& image, ErrorList* result = nullptr);
    void setSample(const cv::Mat& sample, ErrorList* result = nullptr);
    void setSample(const QString& sample, ErrorList* result = nullptr);
    void setMask(const cv::Mat& mask, ErrorList* result = nullptr);
    void setMask(const QString& mask, ErrorList* result = nullptr);
    cv::Mat getMatObject();
    void getImageFromLibrary(const QString& path, cv::Mat& result);

    // --- Delegated: InputService ---
    void clickEsc(ErrorList* result = nullptr, int count = 1);
    void clickReturn(ErrorList* result = nullptr, int count = 2);
    void click(ErrorList* result = nullptr, int count = 10, int delay = 100);
    void clickPosition(const cv::Rect& point, ErrorList* result = nullptr,
                       int count = 10, int delay = 100);
    void clickSwipe(const cv::Rect& start, const cv::Rect& finish,
                    ErrorList* result = nullptr);

    // --- Delegated: GameStateManager ---
    void isEmpty(ErrorList* result = nullptr);
    void isValidSize(ErrorList* result = nullptr);
    void isValidPos(ErrorList* result = nullptr);
    void checkLoading();
    void checkGameLoading();
    void checkPreMainPage();
    void checkMainPage(ErrorList* result = nullptr);
    void checkEvent(ErrorList* result = nullptr);
    void checkSettings(ErrorList* result = nullptr);

    // --- Delegated: GameNavigator ---
    void checkMap(ErrorList* result = nullptr);
    void clickButton(const QString& pagePath, const QString& buttonName,
                     ErrorList* result = nullptr, int count = 3, int delay = 100);
    void clickMapButton(const QString& pageName, const QString& buttonName,
                        ErrorList* result = nullptr, int count = 3, int delay = 100);
    void openAnySets(ErrorList* result = nullptr);

    // --- Delegated: GameFixer ---
    void fixGameError(ErrorList* result = nullptr);
    void fixPopUpError(ErrorList* result = nullptr);
    void fixBattleSettings();
    void refreshMainPage(ErrorList* result = nullptr);
    void skipEvent();

    /// \brief Fixers
    bool fixSimpleError();
    void fixDifficultError();
    void fixIncorrectImageError();
    void fixErrors();

    // --- Delegated: HeroManager ---
    void openHeroPage(ErrorList* result = nullptr);
    void setHeroSet(typeSet set = typeSet::NOT_TOUCH, ErrorList* result = nullptr);
    void setHeroRelics(ErrorList* result = nullptr,
                       const QList<QString>* relicsList = nullptr);

    // --- Delegated: BarracksManager ---
    void findBarracks(ErrorList* result = nullptr);
    void entryBarracks(ErrorList* result = nullptr);
    void scanSquadCount(userProfile* user, ErrorList* result = nullptr);
    void setUnitSet(int index, typeSet set = typeSet::NOT_TOUCH, ErrorList* result = nullptr);

    // --- Delegated: UserManager ---
    void userInitialize(userProfile* user, ErrorList* result = nullptr);

    // --- Config ---
    void setMainPath(const QString& path);
    QString& getMainPath();
    cv::Rect& getRect();

    // --- Debug ---
    void getGameError();

public slots:
    void Start(userProfile* user, ErrorList* result = nullptr);
    void Stop();
    void LocalLogging(const QString& msg);

signals:
    void Recognize(const cv::Mat& object, int& number);
    void emulatorStart(ErrorList* result = nullptr);
    void emulatorStop(HWND* main);
    void emulatorRefresh();
    void Logging(const QString& msg, bool print = true);
    void errorLogging(const QString& msg);
    void endStart();
    void stopStart();
    void emulatorCreated(Emulator* emulator);
    void SuccessFix();
    void saveTaskQueue(int index);

private:
    ControllerState m_state;

    // Services — construction order matches dependency graph (leaves first).
    ImageService    m_images;
    InputService    m_input;
    GameNavigator   m_nav;
    GameStateManager m_checker;
    GameFixer       m_fixer;
    HeroManager     m_hero;
    BarracksManager m_barracks;
    UserManager     m_user;
};
