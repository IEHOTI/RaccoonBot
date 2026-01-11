#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Task/TaskSettings.h"

#include <QObject>

#include <opencv2/opencv.hpp>

#include <windows.h>

class Emulator;

struct ErrorList;

struct userProfile;

struct ImageLibrary;

class BOTLIB_EXPORT Controller : public QObject {
    Q_OBJECT
public:
    /// \brief Controller
    explicit Controller(QObject *parent = nullptr);
    ~Controller();
    void CleanUp();

    // Core

    /// \brief Image_Changers
    //void convertImage(const QImage &imageOne, cv::Mat *imageTwo, ErrorList *result = nullptr);
    cv::Mat cutImage();
    void changeColor(const cv::Mat &before, cv::Mat *after, ErrorList *result = nullptr, bool convert = false);

    /// \brief Image_Finder
    void findObject(const cv::Mat *finder = nullptr, ErrorList *result = nullptr);
    void compareObject(double rightVal = 0.02, const cv::Mat *object = nullptr, const cv::Mat *sample = nullptr, ErrorList *result = nullptr);
    void compareSample(const QString &pagePath, const QString &samplePath, const QString &maskPath, ErrorList *result = nullptr, bool shoot = false, double rightVal = 0.02);

    /// \brief Image_Get_Set_functions
    void Screenshot();
    void saveImage(const QString &savePath, const cv::Mat &saveImage, ErrorList *result = nullptr);
    void setMatObject(const QString &image, ErrorList *result = nullptr);
    void setMatObject(const cv::Mat &image, ErrorList *result = nullptr);
    void setSample(const cv::Mat &sample, ErrorList *result = nullptr);
    void setSample(const QString &sample, ErrorList *result = nullptr);
    void setMask(const cv::Mat &mask, ErrorList *result = nullptr);
    void setMask(const QString &mask, ErrorList *result = nullptr);
    cv::Mat getMatObject();
    void getImageFromLibrary(const QString &path, cv::Mat &result);

    /// \brief Keyboard_click
    void clickEsc(ErrorList *result = nullptr, int count = 1);
    void clickReturn(ErrorList *result = nullptr, int count = 2);

    /// \brief Mouse_click
    void click(ErrorList *result = nullptr, int count = 2, int delay = 100);
    void clickPosition(const cv::Rect &point, ErrorList *result = nullptr, int count = 2, int delay = 100);
    void clickSwipe(const cv::Rect &start,const cv::Rect &finish, ErrorList *result = nullptr);

    // Game

    /// \brief User
    void userInitialize(userProfile *user, ErrorList *result = nullptr);

    /// \brief Get_Set_functions
    void setMainPath(const QString &path);
    QString& getMainPath();
    cv::Rect& getRect();

    /// \brief Fixers
    void fixGameError(ErrorList *result = nullptr);
    void refreshMainPage(ErrorList *result = nullptr);
    void skipEvent();
    void fixPopUpError(ErrorList *result = nullptr);
    void fixBattleSettings();

    /// \brief Checkers
    void isEmpty(ErrorList *result = nullptr);
    void isValidSize(ErrorList *result = nullptr);
    void isValidPos(ErrorList *result = nullptr); // еще подумать над проверкой, так ка кне везде будет 0,34 как в ЛД + добавить проверку m_main чтобы в экране был виден
    void checkMap(ErrorList *result = nullptr);
    void checkLoading();
    void checkGameLoading();
    void checkMainPage(ErrorList *result = nullptr);
    void checkPreMainPage();
    void checkEvent(ErrorList *result = nullptr);
    void checkSettings(ErrorList *result = nullptr);

    /// \brief Common
    void openAnySets(ErrorList *result = nullptr);

    /// \brief Hero
    void openHeroPage(ErrorList *result = nullptr);
    void setHeroSet(typeSet set = typeSet::NOT_TOUCH, ErrorList *result = nullptr);
    void setHeroRelics(ErrorList *result = nullptr, const QList<QString> *relicsList = nullptr);

    /// \brief Barrack
    void findBarracks(ErrorList *result = nullptr);
    void entryBarracks(ErrorList *result = nullptr);
    void scanSquadCount(userProfile *user, ErrorList *result = nullptr);
    void setUnitSet(int index, typeSet set = typeSet::NOT_TOUCH, ErrorList *result = nullptr);

    /// \brief Button_click
    void clickButton(const QString &pagePath,const QString &buttonName, ErrorList *result = nullptr, int count = 2, int delay = 100);
    void clickMapButton(const QString &pageName, const QString &buttonName, ErrorList *result = nullptr, int count = 2, int delay = 100); // pagename = sample or sample_right

    /// \brief later
    void getGameError();

public slots:
    /// \brief Controller
    void Start(userProfile *user, ErrorList *result = nullptr);
    void Stop();
    void LocalLogging(const QString &msg);

signals:
    void Recognize(const cv::Mat &object, int &number);
    void emulatorStart(ErrorList *result = nullptr);
    void emulatorStop(HWND *main);
    void emulatorFix(HWND *main, HWND *game);
    void Logging(const QString &msg, const bool print = true);
    void errorLogging(const QString &msg);
    void endStart();
    void stopStart();
    void emulatorCreated(Emulator *emulator);
    void SuccessFix();//если фиксер починил, но произошла перезагрузка эмулятора начать задание заново i--
    void saveTaskQueue(int index);
private:
    HWND m_main, m_game;
    cv::Mat m_object, m_sample, m_mask;
    cv::Rect m_rect; //при поиске в нём корды
    QString mainPath;
    bool isWorking;
    ImageLibrary *lib;
};

#endif // CONTROLLER_H
