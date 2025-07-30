#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QIntValidator>
#include <QThread>
#include <QDateTime>
#include <QFileDialog>

#include <QMessageBox>

#include "dynamiccombobox.h"
#include "Structs.h"
#include "Controller.h"
#include "Ocr.h"
#include "Emulator.h"
#include "TaskSettings.h"
#include "cathedral.h"
#include "generaldata.h"
#include "arena.h"

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
    void createLoggerTab(QWidget* tab, int index);

    //main
    void createMainTab(QWidget* tab, int index);
    void getSettings(int index);

    //general
    void createGeneralTab(QWidget* tab, int index);
    void readSettings(QString &path, QListWidget *widget);

    //cathedral
    void createCathedralTab(QWidget* tab, int index);

    //arena
    void createArenaTab(QWidget *tab, int index);
signals:
    void startCathedral(bool *result = nullptr); // НЕ НУЖЕН?
    //tab

    //main
    void saveTaskQueue(); // main->general
    void startBot();
    void startController(userProfile* user,bool* result = nullptr);
    //general

    //arena
    void initArena(TaskSettings *setting, bool *result = nullptr);
    void getArenaSettings();

    //cathedral
    void initCathedral(TaskSettings *setting, bool *result = nullptr);
    void getCathedralSettings();//signal from [=]slot on page
private:
    QWidget* mainWidget;
    QList<QString> nameTasks;
    // QList<Task*> listTasks;

    QTabWidget *tabWidget;
    //QVector<QTextEdit*> userLoggers;

    // userProfile *user;
    // Emulator *emulator;
    // QWidget *accountInfo;
    // Controller *controller;
    // QTextEdit *errorLog;
    // Ocr *ocr;

    QList<QThread*> botThreads;
    QList<QSharedPointer<GeneralData>> listData;
    QList<QTextEdit*> listErrorLogs;
    QHash<QString,int> hashTask;
    QSharedPointer<GeneralData> tempData;

    //CathedralSettings *cathedralSettings;
    //Task *cathedral;

    //QThread *controllerThread;
};


#endif // MAINWINDOW_H
