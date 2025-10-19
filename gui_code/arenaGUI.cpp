#include "mainwindow.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QMenu>
#include <QGridLayout>
#include <QWidgetAction>

#include "Arena/Arena.h"
#include "Arena/ArenaSettings.h"
#include "Controller/Controller.h"
#include "User/UserProfile.h"

#include "generaldata.h"

void MainWindow::createArenaTab(QWidget *tab,int index) {
    QCheckBox* adsTask = new QCheckBox("Смотреть рекламу",tab);
    adsTask->setGeometry(5,0,175,20);

    QCheckBox* chestTask = new QCheckBox("Автооткрытие сундуков",tab);
    chestTask->setGeometry(190,0,185,20);

    QRadioButton* bestButton = new QRadioButton("Выставить лучших",tab);
    bestButton->setGeometry(5,25,175,20);

    QRadioButton* lastButton = new QRadioButton("Предыдущий отряд",tab);
    lastButton->setGeometry(5,50,175,20);
    lastButton->setChecked(true);

    QRadioButton* barrackButton = new QRadioButton("Отряд с казармы",tab);
    barrackButton->setGeometry(5,75,175,20);

    QButtonGroup* squad = new QButtonGroup(tab);
    squad->addButton(bestButton);
    squad->addButton(lastButton);
    squad->addButton(barrackButton);

    QComboBox *algorithmBox = new QComboBox(tab);
    algorithmBox->setGeometry(190,25,155,20);
    algorithmBox->addItem("Cтабильный алгоритм");
    algorithmBox->addItem("Умный алгоритм");
    algorithmBox->addItem("Свой алгоритм");

    QPushButton *setAlgorithm = new QPushButton(tab);
    setAlgorithm->setGeometry(350,25,20,20);
    setAlgorithm->setEnabled(false);

    connect(algorithmBox,&QComboBox::currentIndexChanged,this,[=](){
        if(algorithmBox->currentIndex() == 2) setAlgorithm->setEnabled(true);
        else setAlgorithm->setEnabled(false);
    });

    QMenu *setMenu = new QMenu(tab);
    QWidget *setWidget = new QWidget();
    QGridLayout *setLayout = new QGridLayout(setWidget);
    QComboBox *setPhaseOneBox = new QComboBox(setWidget);
    setPhaseOneBox->addItem("Выше себя");
    setPhaseOneBox->addItem("Ниже себя");
    QComboBox *setPhaseTwoBox = new QComboBox(setWidget);
    setPhaseTwoBox->addItem("Выше себя");
    setPhaseTwoBox->addItem("Ниже себя");
    QComboBox *setPhaseThreeBox = new QComboBox(setWidget);
    setPhaseThreeBox->addItem("Выше себя");
    setPhaseThreeBox->addItem("Ниже себя");
    QComboBox *setPhaseFourBox = new QComboBox(setWidget);
    setPhaseFourBox->addItem("Выше себя");
    setPhaseFourBox->addItem("Ниже себя");
    QComboBox *setPhaseFiveBox = new QComboBox(setWidget);
    setPhaseFiveBox->addItem("Выше себя");
    setPhaseFiveBox->addItem("Ниже себя");

    setLayout->addWidget(new QLabel("Первая фаза"),0,0);
    setLayout->addWidget(setPhaseOneBox,0,1);
    setLayout->addWidget(new QLabel("Вторая фаза"),1,0);
    setLayout->addWidget(setPhaseTwoBox,1,1);
    setLayout->addWidget(new QLabel("Третья фаза"),2,0);
    setLayout->addWidget(setPhaseThreeBox,2,1);
    setLayout->addWidget(new QLabel("Четвертая фаза"),3,0);
    setLayout->addWidget(setPhaseFourBox,3,1);
    setLayout->addWidget(new QLabel("Пятая фаза"),4,0);
    setLayout->addWidget(setPhaseFiveBox,4,1);
    setWidget->setLayout(setLayout);
    QWidgetAction *setAlgorithmWidget = new QWidgetAction(setMenu);
    setAlgorithmWidget->setDefaultWidget(setWidget);
    setMenu->addAction(setAlgorithmWidget);

    setAlgorithm->setMenu(setMenu);

    QLabel* powerLabel = new QLabel("Множитель мощи",tab);
    powerLabel->setGeometry(190,50,140,20);

    QLineEdit* powerLine = new QLineEdit(tab);
    powerLine->setGeometry(340,50,35,20);
    powerLine->setText("1.0");
    QDoubleValidator* powerValid = new QDoubleValidator(0.0,9.9,1,tab);
    powerValid->setNotation(QDoubleValidator::StandardNotation);
    powerValid->setLocale(QLocale::C);
    powerLine->setValidator(powerValid);

    connect(powerLine, &QLineEdit::textEdited, this, [powerLine]() {
        QString text = powerLine->text();
        if (text.contains(',')) {
            text.replace(',', '.');
            powerLine->setText(text);
        }
    });

    QCheckBox *BLBox = new QCheckBox("BlackList",tab);
    BLBox->setGeometry(190,75,90,20);
    BLBox->setChecked(true);

    QCheckBox *WLBox = new QCheckBox("WhiteList",tab);
    WLBox->setGeometry(285,75,90,20);
    WLBox->setChecked(true);

    QLabel* resourceLabel = new QLabel("Расходовать",tab);
    resourceLabel->setGeometry(190,100,90,20);

    QComboBox* resourceBox = new QComboBox(tab);
    resourceBox->setGeometry(275,100,100,20);
    resourceBox->addItem("Яблоки");
    resourceBox->addItem("Билеты");
    resourceBox->addItem("Яблоки > Билеты");
    resourceBox->addItem("Билеты > Яблоки");

    QLabel* countLabel = new QLabel("Количество турниров",tab);
    countLabel->setGeometry(190,125,150,20);

    QComboBox* countBox = new QComboBox(tab);
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
    foodLabel->setGeometry(190,150,130,20);

    QLineEdit* foodLine = new QLineEdit(tab);
    foodLine->setGeometry(305,150,70,20);
    foodLine->setText("0");
    QIntValidator* foodValid = new QIntValidator(0, 2000000,tab);
    foodLine->setValidator(foodValid);

    ////////////
    QWidget *unitFirstWidget = new QWidget(tab);
    createUnitsWidget(tab,unitFirstWidget);
    unitFirstWidget->setEnabled(false);

    connect(barrackButton,&QRadioButton::toggled, this, [=](bool checked){
        if(checked) unitFirstWidget->setEnabled(true);
        else unitFirstWidget->setEnabled(false);
    });
    ///////

    int fixedIndex = index;
    connect(this,&MainWindow::getArenaSettings,this,[=](int local_index){
        if(local_index != fixedIndex) return;
        tab->setEnabled(false);
        userProfile *tempUser = listData[fixedIndex]->user;
        ArenaSettings *settings = new ArenaSettings();
        settings->history_power = tempUser->history_power;
        QString str = countBox->currentText();
        if (str == "∞") settings->count = -1;
        else settings->count = str.toInt();
        settings->premiumStatus = tempUser->state_premium;
        settings->watchADS = adsTask->isChecked();
        if(bestButton->isChecked()) settings->modeSquad = 0;
        else if(lastButton->isChecked()) settings->modeSquad = 1;
        else if(barrackButton->isChecked()) {
            settings->modeSquad = 2;
            for(int i = 0; i < 8; i++) {
                typeSet temp;
                emit getUnitSet(tab,i,temp);
                settings->squadSet.append(temp);
            }
        }
        settings->modeTicket = resourceBox->currentIndex();
        settings->openChest = chestTask->isChecked();
        int temp = foodLine->text().remove(' ').toInt();
        if(temp > 2000000) temp = 2000000;
        else if(temp < 0) temp = 0;
        settings->saveApple = temp;
        double tempp = powerLine->text().toDouble();
        if(tempp > 9.9) tempp = 9.9;
        else if(tempp < 0.0) tempp = 0.0;
        settings->rangePower = tempp;
        switch (algorithmBox->currentIndex()){
        case 0: break;
        case 1:{
            settings->strategy.fourth = true;
            settings->strategy.fives = true;
            break;
        }
        case 2:{
            if(setPhaseOneBox->currentIndex() == 0) settings->strategy.first = true;
            else settings->strategy.first = false;
            if(setPhaseTwoBox->currentIndex() == 0) settings->strategy.second = true;
            else settings->strategy.second = false;
            if(setPhaseThreeBox->currentIndex() == 0) settings->strategy.third = true;
            else settings->strategy.third = false;
            if(setPhaseFourBox->currentIndex() == 0) settings->strategy.fourth = true;
            else settings->strategy.fourth = false;
            if(setPhaseFiveBox->currentIndex() == 0) settings->strategy.fives = true;
            else settings->strategy.fives = false;
            break;
        }
        default:{
            //err
            emit listData[fixedIndex]->controller->errorLogging("Ошибка установки алгоритма арены. Установлен стандарт.");
            break;
        }
        }
        settings->strategy.black = BLBox->isChecked();
        settings->strategy.white = WLBox->isChecked();

        Arena *task = new Arena(listData[fixedIndex]->controller);
        task->moveToThread(botThreads[fixedIndex]);
        QMetaObject::invokeMethod(task, [task, parent=listData[fixedIndex].data()](){
            task->setParent(parent);
        }, Qt::QueuedConnection);
        connect(this,&MainWindow::initArena,task,&Arena::Initialize,Qt::QueuedConnection);
        emit initArena(settings);
        listData[fixedIndex]->listTasks.append(task);
        listData[fixedIndex]->listSettings.append(settings);

        tab->setEnabled(true);
    });
}
