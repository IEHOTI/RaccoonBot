#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QListWidget>
#include "qcontainerfwd.h"
#include "Task/TaskSettings.h"

struct GeneralData;
struct userProfile;
struct ErrorList;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setCentralWindow(int width, int height);
    void preProcessingData(); // возможно тут сделать потом проверку на загрузку данных после закрытия
    void preProcessingMenuBar();

    //tab
    void createGUI();
    void createLoggerTab(QWidget *tab, int index);
    void createUnitsWidget(QWidget *mainTab, QWidget *widget);
    void createHeroWidget(QWidget *mainTab, QWidget *widget);
    //main
    void createMainTab(QWidget *tab, int index);
    void getSettings(int index);

    //general
    void createGeneralTab(QWidget *tab, int index);
    void readSettings(QString &path, QListWidget *widget);

    //cathedral
    void createCathedralTab(QWidget *tab, int index);

    //arena
    void createArenaTab(QWidget *tab, int index);

    //lighthouse
    void createLighthouseTab(QWidget *tab, int index);
signals:
    //tab
    void getUnitSet(QWidget *tab, int index, typeSet &set);
    void getUnits();
    void getHeroRelics(QWidget *tab, int &set, QStringList &relics);

    //main
    void startBot();
    void startController(userProfile *user, ErrorList *result = nullptr);
    //general

    //arena
    void initArena(TaskSettings *setting, ErrorList *result = nullptr);
    void getArenaSettings(int local_index);

    //cathedral
    void initCathedral(TaskSettings *setting, ErrorList *result = nullptr);
    void getCathedralSettings(int local_index);

    //lighthouse
    void initLighthouse(TaskSettings *setting, ErrorList *result = nullptr);
    void getLighthouseSettings(int local_index);
private:
    QWidget *mainWidget;
    QList<QString> nameTasks;
    QList<QString> usedEmulators;

    QTabWidget *tabWidget;

    QList<QThread*> botThreads;
    QList<QSharedPointer<GeneralData>> listData;
    QList<QTextEdit*> listErrorLogs;
    QHash<QString,int> hashTask;
    QSharedPointer<GeneralData> tempData;
};


#endif // MAINWINDOW_H
