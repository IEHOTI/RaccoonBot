#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QThread>
#include <QCoreApplication>
#include "BotLib_global.h"
#include "Structs.h"
#include "Emulator.h"
#include "ldplayer.h"
#include "EmulatorComponents.h"
#include "Error.h"
#include <QDir>
#include <QFile>

class BOTLIB_EXPORT Controller : public QObject {
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() = default;

    /////
    /// \brief image
    ///
    void findObject(const Mat *finder = nullptr, ErrorList *result = nullptr);
    void compareObject(double rightVal = 0.02, const Mat *object = nullptr, const Mat *sample = nullptr, ErrorList *result = nullptr);
    void compareSample(const QString &pagePath, const QString &samplePath, const QString &maskPath, ErrorList *result = nullptr, bool shoot = false, double rightVal = 0.02);
    void saveImage(const QString &savePath, const Mat &saveImage, ErrorList *result = nullptr);
    void Screenshot();
    void convertImage(const QImage &imageOne, Mat *imageTwo, ErrorList *result = nullptr);
    Mat cutImage();
    Mat getMatObject();
    void setMatObject(const Mat &image, ErrorList *result = nullptr);
    void changeColor(const Mat &before, Mat *after, ErrorList *result = nullptr); //Mat ** из-за гонки потоков, лол?
    void setSample(const Mat &sample, ErrorList *result = nullptr);
    void setSample(const QString &sample, ErrorList *result = nullptr);
    void setMask(const Mat &mask, ErrorList *result = nullptr);
    void setMask(const QString &mask, ErrorList *result = nullptr);
    Rect &getRect();
    /////
    /// \brief emulator
    ///
    void isEmpty(ErrorList *result = nullptr);
    void isValidSize(ErrorList *result = nullptr);
    void isValidPos(ErrorList *result = nullptr); // еще подумать над проверкой, так ка кне везде будет 0,34 как в ЛД + добавить проверку m_main чтобы в экране был виден
    /////
    /// \brief keyboard + mouse
    ///
    void click(ErrorList *result = nullptr, int count = 2, int delay = 100);
    void clickPosition(const Rect &point, ErrorList *result = nullptr, int count = 2, int delay = 100);
    void clickSwipe(const Rect &start,const Rect &finish, ErrorList *result = nullptr);
    void clickButton(const QString &pagePath,const QString &buttonName, ErrorList *result = nullptr, int count = 2, int delay = 100);
    void clickEsc(ErrorList *result = nullptr, int count = 1);
    void clickReturn(ErrorList *result = nullptr, int count = 2);
    void clickMapButton(const QString &pageName, const QString &buttonName, ErrorList *result = nullptr, int count = 2, int delay = 100); // pagename = sample or sample_right
    /////
    /// \brief game
    ///
    //void Initialize(); // функция будет из файла подтягивать юзеров
    void userInitialize(userProfile *user, ErrorList *result = nullptr);
    void fixGameError(ErrorList *result = nullptr);
    void getGameError();
    void refreshMainPage(ErrorList *result = nullptr);
    void skipEvent();
    void fixPopUpError(ErrorList *result = nullptr);
    ////
    /// \brief barracks
    ///
    void findBarracks(ErrorList *result = nullptr);
    void entryBarracks(ErrorList *result = nullptr);
    void scanSquadCount(userProfile *user, ErrorList *result = nullptr);
    void setUnitSet(int index, typeSet set = typeSet::NOT_TOUCH, ErrorList *result = nullptr);
    ///
    /// \brief game checker доделать
    ///
    void checkMap(ErrorList *result = nullptr);
    void checkLoading();
    void checkGameLoading();
    void checkMainPage(ErrorList *result = nullptr);
    void checkPreMainPage();
    void checkEvent(ErrorList *result = nullptr);
    void checkSettings(ErrorList *result = nullptr);
    /////
    /// \brief microTasks вынести в отдельный класс наследник от тасков обычных
    ///
    //void takeEmojis(ErrorList *result = nullptr);
    /////
    /// \brief path
    ///
    QString& getMainPath();
    void setMainPath(const QString &path);
    /////
    /// \brief controller
    ///
    void CleanUp();

public slots:
    void Start(userProfile *user, ErrorList *result = nullptr);
    void Stop();
    void LocalLogging(const QString &msg);
signals:
    void Recognize(const Mat &object, int &number);
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
    Mat m_object, m_sample, m_mask;
    Rect m_rect; //при поиске в нём корды
    QString mainPath;
    bool isWorking;
};

#endif // CONTROLLER_H
