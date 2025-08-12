#ifndef CONTROLLER_H
#define CONTROLLER_H


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
    explicit Controller(QObject *parent = nullptr);  // Добавляем параметр parent
    ~Controller() = default;

    //Для тестов(потом удалить)
    void InitLight(userProfile user);
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
    void click(ErrorList *result = nullptr, int count = 2, int delay = 200);
    void clickPosition(const Rect &point, ErrorList *result = nullptr, int count = 2, int delay = 200);
    void clickSwipe(const Rect &start,const Rect &finish, ErrorList *result = nullptr);
    void clickButton(const QString &pagePath,const QString &buttonName, ErrorList *result = nullptr, int count = 2, int delay = 200);
    void clickEsc(ErrorList *result = nullptr);
    void clickReturn(ErrorList *result = nullptr);
    void clickMapButton(const QString &pageName, const QString &buttonName, ErrorList *result = nullptr, int count = 2, int delay = 200); // pagename = sample or sample_right
    /////
    /// \brief game
    ///
    //void Initialize(); // функция будет из файла подтягивать юзеров
    void userInitialize(userProfile *user, ErrorList *result = nullptr);
    void fixGameError(ErrorList *result = nullptr);
    void getGameError();
    void refreshMainPage(ErrorList *result = nullptr);// доделать, не распознает правильно
    void skipEvent();
    void fixPopUpError(ErrorList *result = nullptr);
    ////
    /// \brief barracks
    ///
    void findBarracks(ErrorList *result = nullptr);//
    void entryBarracks(ErrorList *result = nullptr);//
    void scanSquadCount(userProfile *user, ErrorList *result = nullptr);//
    ///
    /// \brief game checker доделать
    ///
    void checkMap(ErrorList *result = nullptr);//сделать
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
    void Start(userProfile *user, ErrorList *result = nullptr); // тут настройки проверяются в игре и сама игра дописать
    void Stop(ErrorList *result = nullptr);
    void LocalLogging(const QString &msg); // внутри тупо emit errorLogging();
signals:
    void Recognize(const Mat &object, int &number);
    void emulatorStart(ErrorList *result = nullptr);
    void emulatorStop(HWND *main);
    void emulatorFix(HWND *main, HWND *game);
    void Logging(const QString &msg, const bool print = true);
    void errorLogging(const QString &msg);
    void endStart();
    void emulatorCreated(Emulator *emulator);
    void SuccessFix();//если фиксер починил, но произошла перезагрузка эмулятора начать задание заново i--
private:
    HWND m_main, m_game;
    Mat m_object, m_sample, m_mask;
    Rect m_rect; //при поиске в нём корды
    QString mainPath;
};

#endif // CONTROLLER_H
