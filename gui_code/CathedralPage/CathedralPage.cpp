#include "MainWindow/MainWindow.h"
#include "MyClasses/GeneralData.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>

#include "Cathedral/CathedralSettings.h"
#include "Cathedral/Cathedral.h"
#include "User/UserProfile.h"

void MainWindow::createCathedralTab(QWidget *tab,int index) {
    QCheckBox* adsTask = new QCheckBox("Смотреть рекламу",tab);
    adsTask->setObjectName("adsTask");
    adsTask->setGeometry(5,0,175,20);

    QCheckBox* chestTask = new QCheckBox("Автооткрытие сундуков",tab);
    chestTask->setObjectName("chestTask");
    chestTask->setGeometry(175,0,200,20);

    QRadioButton* bestButton = new QRadioButton("Выставить лучших",tab);
    bestButton->setObjectName("bestButton");
    bestButton->setGeometry(5,25,175,20);

    QRadioButton* lastButton = new QRadioButton("Предыдущий отряд",tab);
    lastButton->setObjectName("lastButton");
    lastButton->setGeometry(5,50,175,20);
    lastButton->setChecked(true);

    QRadioButton* barrackButton = new QRadioButton("Отряд с казармы",tab);
    barrackButton->setObjectName("barrackButton");
    barrackButton->setGeometry(5,75,175,20);

    QButtonGroup* squad = new QButtonGroup(tab);
    squad->setObjectName("squad");
    squad->addButton(bestButton);
    squad->addButton(lastButton);
    squad->addButton(barrackButton);

    QRadioButton* fullButton = new QRadioButton("Полное прохождение",tab);
    fullButton->setObjectName("fullButton");
    fullButton->setGeometry(175,25,200,20);
    fullButton->setChecked(true);

    QRadioButton* fastButton = new QRadioButton("Боссы и миньоны",tab);
    fastButton->setObjectName("fastButton");
    fastButton->setGeometry(175,50,200,20);

    QButtonGroup* mode = new QButtonGroup(tab);
    mode->setObjectName("mode");
    mode->addButton(fullButton);
    mode->addButton(fastButton);

    QLabel* modeLabel = new QLabel("Сложность",tab);
    modeLabel->setObjectName("modeLabel");
    modeLabel->setGeometry(175,75,105,20);

    QComboBox* modeBox = new QComboBox(tab);
    modeBox->setObjectName("modeBox");
    modeBox->setGeometry(275,75,100,20);
    modeBox->addItem("Нормальная");
    modeBox->addItem("Сложная");
    modeBox->addItem("Кошмарная");

    QLabel* resourceLabel = new QLabel("Расходовать",tab);
    resourceLabel->setObjectName("resourceLabel");
    resourceLabel->setGeometry(175,100,105,20);

    QComboBox* resourceBox = new QComboBox(tab);
    resourceBox->setObjectName("resourceBox");
    resourceBox->setGeometry(275,100,100,20);
    resourceBox->addItem("Яблоки");
    resourceBox->addItem("Ключи");
    resourceBox->addItem("Яблоки > Ключи");
    resourceBox->addItem("Ключи > Яблоки");

    QLabel* countLabel = new QLabel("Количество походов",tab);
    countLabel->setObjectName("countLabel");
    countLabel->setGeometry(175,125,165,20);

    QComboBox* countBox = new QComboBox(tab);
    countBox->setObjectName("countBox");
    countBox->setGeometry(335,125,40,20);
    countBox->addItem("1");
    countBox->addItem("2");
    countBox->addItem("3");
    countBox->addItem("4");
    countBox->addItem("5");
    countBox->addItem("7");
    countBox->addItem("10");
    countBox->addItem("20");
    countBox->addItem("30");
    countBox->addItem("∞");

    QLabel* foodLabel = new QLabel("Сохранять еды",tab);
    foodLabel->setObjectName("foodLabel");
    foodLabel->setGeometry(175,150,130,20);

    QLineEdit* foodLine = new QLineEdit(tab);
    foodLine->setObjectName("foodLine");
    foodLine->setGeometry(305,150,70,20);
    foodLine->setText("0");
    QIntValidator* foodValid = new QIntValidator(0, 2000000,tab);
    foodLine->setValidator(foodValid);

    int fixedIndex = index;
    connect(this,&MainWindow::getCathedralSettings,this,[=](int local_index){
        if(local_index != fixedIndex) return;
        tab->setEnabled(false);
        userProfile *tempUser = listData[fixedIndex]->user;
        CathedralSettings* settings = new CathedralSettings();
        settings->history_power = tempUser->history_power;
        QString str = countBox->currentText();
        if (str == "∞") settings->count = -1;
        else settings->count = str.toInt();
        settings->premiumStatus = tempUser->state_premium;
        settings->watchADS = adsTask->isChecked();
        //
        settings->modeDifficult = modeBox->currentIndex();
        if(bestButton->isChecked()) settings->modeSquad = 0;
        if(lastButton->isChecked()) settings->modeSquad = 1;
        if(barrackButton->isChecked()) settings->modeSquad = 2;
        settings->modeKey = resourceBox->currentIndex();
        settings->fullGamePass = fullButton->isChecked();
        settings->openChest = chestTask->isChecked();
        int temp = foodLine->text().remove(' ').toInt();
        if(temp > 2000000) temp = 2000000;
        else if(temp < 0) temp = 0;
        settings->saveApple = temp;
        Cathedral *task = new Cathedral(listData[fixedIndex]->controller);
        task->moveToThread(botThreads[fixedIndex]);
        QMetaObject::invokeMethod(task, [task, parent=listData[fixedIndex].data()](){
            task->setParent(parent);
        }, Qt::QueuedConnection);
        connect(this,&MainWindow::initCathedral,task,&Cathedral::Initialize,Qt::QueuedConnection);
        emit initCathedral(settings);
        listData[fixedIndex]->listTasks.append(task);
        listData[fixedIndex]->listSettings.append(settings);

        tab->setEnabled(true);
    });
}
