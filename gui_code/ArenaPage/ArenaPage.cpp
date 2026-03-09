#include "MainWindow/MainWindow.h"
#include "MyClasses/GeneralData.h"

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

void MainWindow::createArenaTab(QWidget *tab,int index) {
    QHBoxLayout *mainLayout = new QHBoxLayout(tab);
    mainLayout->setObjectName("arenaMainLayout");

    // left start
    QWidget *leftWidget = new QWidget(tab);
    leftWidget->setObjectName("arenaLeftWidget");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setObjectName("arenaLeftLayout");
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    QWidget *algorithmWidget = new QWidget(leftWidget);
    algorithmWidget->setObjectName("arenaAlgorithmWidget");
    algorithmWidget->setFixedHeight(40);
    QHBoxLayout *algorithmLayout = new QHBoxLayout(algorithmWidget);
    algorithmLayout->setObjectName("arenaAlgorithmLayout");
    algorithmLayout->setContentsMargins(0, 0, 0, 0);

    QComboBox *algorithmBox = new QComboBox(algorithmWidget);
    algorithmBox->setObjectName("algorithmBox");
    algorithmBox->addItems({"Стабильный алгоритм","Умный алгоритм","Свой алгоритм"});
    algorithmLayout->addWidget(algorithmBox);

    QPushButton *algorithmButton = new QPushButton(algorithmWidget);
    algorithmButton->setObjectName("algorithmButton");
    algorithmButton->setFixedWidth(30);
    algorithmButton->setEnabled(false);
    algorithmLayout->addWidget(algorithmButton);

    connect(algorithmBox, &QComboBox::currentIndexChanged, this, [=](int index){
        algorithmButton->setEnabled(index == 2);
    });

    QMenu *algorithmMenu = new QMenu(algorithmWidget);
    algorithmMenu->setObjectName("algorithmMenu");

    QWidget *algorithmSetWidget = new QWidget(algorithmWidget);
    algorithmSetWidget->setObjectName("algorithmSetWidget");
    QGridLayout *algorithmSetLayout = new QGridLayout(algorithmSetWidget);
    algorithmSetLayout->setObjectName("algorithmSetLayout");

    QStringList phaseItems{"Выше себя","Ниже себя"};

    QComboBox *phaseOneBox = new QComboBox(algorithmSetWidget);
    phaseOneBox->setObjectName("phaseOneBox");
    phaseOneBox->addItems(phaseItems);
    algorithmSetLayout->addWidget(new QLabel("Первая фаза"), 0, 0);
    algorithmSetLayout->addWidget(phaseOneBox, 0, 1);

    QComboBox *phaseTwoBox = new QComboBox(algorithmSetWidget);
    phaseTwoBox->setObjectName("phaseTwoBox");
    phaseTwoBox->addItems(phaseItems);
    algorithmSetLayout->addWidget(new QLabel("Вторая фаза"), 1, 0);
    algorithmSetLayout->addWidget(phaseTwoBox, 1, 1);

    QComboBox *phaseThreeBox = new QComboBox(algorithmSetWidget);
    phaseThreeBox->setObjectName("phaseThreeBox");
    phaseThreeBox->addItems(phaseItems);
    algorithmSetLayout->addWidget(new QLabel("Третья фаза"), 2, 0);
    algorithmSetLayout->addWidget(phaseThreeBox, 2, 1);

    QComboBox *phaseFourBox = new QComboBox(algorithmSetWidget);
    phaseFourBox->setObjectName("phaseFourBox");
    phaseFourBox->addItems(phaseItems);
    algorithmSetLayout->addWidget(new QLabel("Четвертая фаза"), 3, 0);
    algorithmSetLayout->addWidget(phaseFourBox, 3, 1);

    QComboBox *phaseFiveBox = new QComboBox(algorithmSetWidget);
    phaseFiveBox->setObjectName("phaseFiveBox");
    phaseFiveBox->addItems(phaseItems);
    algorithmSetLayout->addWidget(new QLabel("Пятая фаза"), 4, 0);
    algorithmSetLayout->addWidget(phaseFiveBox, 4, 1);

    QWidgetAction *algorithmAction = new QWidgetAction(algorithmMenu);
    algorithmAction->setObjectName("algorithmAction");
    algorithmAction->setDefaultWidget(algorithmSetWidget);
    algorithmMenu->addAction(algorithmAction);
    algorithmButton->setMenu(algorithmMenu);

    leftLayout->addWidget(algorithmWidget);

    QWidget *squadWidget = new QWidget(leftWidget);
    squadWidget->setObjectName("arenaSquadWidget");
    QVBoxLayout *squadLayout = new QVBoxLayout(squadWidget);
    squadLayout->setObjectName("arenaSquadLayout");
    squadLayout->setContentsMargins(0, 0, 0, 0);

    QRadioButton *bestButton = new QRadioButton("Выставить лучших", squadWidget);
    bestButton->setObjectName("bestButton");
    bestButton->setFixedHeight(25);
    squadLayout->addWidget(bestButton);

    QRadioButton *lastButton = new QRadioButton("Предыдущий отряд", squadWidget);
    lastButton->setObjectName("lastButton");
    lastButton->setFixedHeight(25);
    lastButton->setChecked(true);
    squadLayout->addWidget(lastButton);

    QWidget *barrackWidget = new QWidget(squadWidget);
    barrackWidget->setObjectName("arenaBarrackWidget");
    barrackWidget->setFixedHeight(25);
    QHBoxLayout *barrackLayout = new QHBoxLayout(barrackWidget);
    barrackLayout->setObjectName("arenaBarrackLayout");
    barrackLayout->setContentsMargins(0, 0, 0, 0);

    QRadioButton *barrackButton = new QRadioButton("Отряд с казармы", barrackWidget);
    barrackButton->setObjectName("barrackButton");
    barrackLayout->addWidget(barrackButton);

    QPushButton *unitsButton = new QPushButton(barrackWidget);
    unitsButton->setObjectName("arenaUnitsButton");
    unitsButton->setFixedWidth(30);
    barrackLayout->addWidget(unitsButton);

    QMenu *unitsMenu = new QMenu(barrackWidget);
    unitsMenu->setObjectName("arenaUnitsMenu");

    QWidget *unitsWidget = new QWidget(barrackWidget);
    unitsWidget->setObjectName("arenaUnitsWidget");
    createUnitsWidget(tab, unitsWidget);

    QWidgetAction *unitsAction = new QWidgetAction(unitsMenu);
    unitsAction->setObjectName("arenaUnitsAction");
    unitsAction->setDefaultWidget(unitsWidget);
    unitsMenu->addAction(unitsAction);
    unitsButton->setMenu(unitsMenu);
    unitsButton->setEnabled(false);

    connect(barrackButton, &QRadioButton::toggled, this, [=](bool checked){
        unitsButton->setEnabled(checked);
    });

    squadLayout->addWidget(barrackWidget);

    QButtonGroup *squadGroup = new QButtonGroup(squadWidget);
    squadGroup->setObjectName("squadGroup");
    squadGroup->addButton(bestButton);
    squadGroup->addButton(lastButton);
    squadGroup->addButton(barrackButton);

    leftLayout->addWidget(squadWidget);

    QWidget *additionWidget = new QWidget(leftWidget);
    additionWidget->setObjectName("arenaAdditionWidget");
    QVBoxLayout *additionLayout = new QVBoxLayout(additionWidget);
    additionLayout->setObjectName("arenaAdditionLayout");
    additionLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *heroWidget = new QWidget(additionWidget);
    heroWidget->setObjectName("arenaHeroWidget");
    createHeroWidget(tab, heroWidget);
    heroWidget->setEnabled(true);
    additionLayout->addWidget(heroWidget);

    QWidget *petsWidget = new QWidget(additionWidget);
    petsWidget->setObjectName("arenaPetsWidget");
    createPetsWidget(tab, petsWidget);
    petsWidget->setEnabled(true);
    additionLayout->addWidget(petsWidget);

    QWidget *titansWidget = new QWidget(additionWidget);
    titansWidget->setObjectName("arenaTitansWidget");
    createTitanWidget(tab, titansWidget);
    titansWidget->setEnabled(true);
    additionLayout->addWidget(titansWidget);

    leftLayout->addWidget(additionWidget);
    leftWidget->setMinimumWidth(190);
    // left end

    // right start
    QWidget *rightWidget = new QWidget(tab);
    rightWidget->setObjectName("arenaRightWidget");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setObjectName("arenaRightLayout");
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QWidget *checkBoxWidget = new QWidget(rightWidget);
    checkBoxWidget->setObjectName("arenaCheckBoxWidget");
    checkBoxWidget->setFixedHeight(120);
    QVBoxLayout *checkBoxLayout = new QVBoxLayout(checkBoxWidget);
    checkBoxLayout->setObjectName("arenaCheckBoxLayout");
    checkBoxLayout->setContentsMargins(0, 0, 0, 0);
    checkBoxLayout->setSpacing(0);

    QCheckBox *adsTask = new QCheckBox("Смотреть рекламу", checkBoxWidget);
    adsTask->setObjectName("adsTask");
    adsTask->setFixedHeight(40);
    checkBoxLayout->addWidget(adsTask);

    QCheckBox *chestTask = new QCheckBox("Автооткрытие сундуков", checkBoxWidget);
    chestTask->setObjectName("chestTask");
    chestTask->setFixedHeight(40);
    checkBoxLayout->addWidget(chestTask);

    QWidget *listsWidget = new QWidget(checkBoxWidget);
    listsWidget->setObjectName("arenaListsWidget");
    listsWidget->setFixedHeight(40);
    QHBoxLayout *listsLayout = new QHBoxLayout(listsWidget);
    listsLayout->setObjectName("arenaListsLayout");
    listsLayout->setContentsMargins(0, 0, 0, 0);
    listsLayout->setSpacing(0);

    QCheckBox *BLBox = new QCheckBox("BlackList", listsWidget);
    BLBox->setObjectName("BLBox");
    BLBox->setChecked(true);
    listsLayout->addWidget(BLBox);

    QCheckBox *WLBox = new QCheckBox("WhiteList", listsWidget);
    WLBox->setObjectName("WLBox");
    WLBox->setChecked(true);
    listsLayout->addWidget(WLBox);

    checkBoxLayout->addWidget(listsWidget);
    rightLayout->addWidget(checkBoxWidget);

    QWidget *powerWidget = new QWidget(rightWidget);
    powerWidget->setObjectName("arenaPowerWidget");
    powerWidget->setFixedHeight(40);
    QHBoxLayout *powerLayout = new QHBoxLayout(powerWidget);
    powerLayout->setObjectName("arenaPowerLayout");
    powerLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *powerLabel = new QLabel("Множитель мощи", powerWidget);
    powerLayout->addWidget(powerLabel);

    QLineEdit *powerLine = new QLineEdit(powerWidget);
    powerLine->setObjectName("powerLine");
    powerLine->setFixedSize(35, 20);
    powerLine->setText("1.0");
    powerLayout->addWidget(powerLine);

    QDoubleValidator *powerValid = new QDoubleValidator(0.0, 9.9, 1, powerWidget);
    powerValid->setObjectName("powerValidator");
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

    rightLayout->addWidget(powerWidget);

    QWidget *consumeWidget = new QWidget(rightWidget);
    consumeWidget->setObjectName("arenaConsumeWidget");
    consumeWidget->setFixedHeight(60);
    QVBoxLayout *consumeLayout = new QVBoxLayout(consumeWidget);
    consumeLayout->setObjectName("arenaConsumeLayout");
    consumeLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *consumeLabel = new QLabel("Используемые расходники", consumeWidget);
    consumeLayout->addWidget(consumeLabel);

    QComboBox *consumeBox = new QComboBox(consumeWidget);
    consumeBox->setObjectName("consumeBox");
    consumeBox->addItems({"Яблоки","Билеты","Яблоки > Билеты", "Билеты > Яблоки"});
    consumeLayout->addWidget(consumeBox);

    rightLayout->addWidget(consumeWidget);

    QWidget *saverWidget = new QWidget(rightWidget);
    saverWidget->setObjectName("arenaSaverWidget");
    saverWidget->setFixedHeight(40);
    QHBoxLayout *saverLayout = new QHBoxLayout(saverWidget);
    saverLayout->setObjectName("arenaSaverLayout");
    saverLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *saverLabel = new QLabel("Сохранять еды", saverWidget);
    saverLayout->addWidget(saverLabel);

    QLineEdit *saverLine = new QLineEdit(saverWidget);
    saverLine->setObjectName("saverLine");
    saverLine->setFixedSize(65, 20);
    saverLine->setText("0");
    saverLayout->addWidget(saverLine);

    QIntValidator *saverValid = new QIntValidator(0, 2000000, saverWidget);
    saverValid->setObjectName("saverValidator");
    saverLine->setValidator(saverValid);

    rightLayout->addWidget(saverWidget);

    QWidget *counterWidget = new QWidget(rightWidget);
    counterWidget->setObjectName("arenaCounterWidget");
    counterWidget->setFixedHeight(40);
    QHBoxLayout *counterLayout = new QHBoxLayout(counterWidget);
    counterLayout->setObjectName("arenaCounterLayout");
    counterLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *counterLabel = new QLabel("Количество турниров", counterWidget);
    counterLayout->addWidget(counterLabel);

    QComboBox *counterBox = new QComboBox(counterWidget);
    counterBox->setObjectName("counterBox");
    counterBox->setFixedSize(40, 20);
    counterBox->addItems({"1","2","3","4","5","7","10","20","30","∞"});
    counterLayout->addWidget(counterBox);

    rightLayout->addWidget(counterWidget);
    // right end

    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(rightWidget);

    int fixedIndex = index;
    connect(this,&MainWindow::getArenaSettings,this,[=](int local_index){
        if(local_index != fixedIndex) return;
        tab->setEnabled(false);
        userProfile *tempUser = listData[fixedIndex]->user;
        ArenaSettings *settings = new ArenaSettings();
        settings->history_power = tempUser->history_power;
        QString str = counterBox->currentText();
        if (str == "∞") settings->count = -1;
        else settings->count = str.toInt();
        settings->premiumStatus = tempUser->state_premium;
        settings->watchADS = adsTask->isChecked();
        if(bestButton->isChecked()) settings->modeSquad = 0;
        else if(lastButton->isChecked()) settings->modeSquad = 1;
        else if(barrackButton->isChecked()) {
            settings->modeSquad = 2;
            for(int i = 0; i < 8; ++i) {
                typeSet temp;
                emit getUnitSet(tab,i,temp);
                settings->squadSet.append(temp);
            }
        }
        settings->modeTicket = consumeBox->currentIndex();
        settings->openChest = chestTask->isChecked();
        int temp = saverLine->text().remove(' ').toInt();
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
            if(phaseOneBox->currentIndex() == 0) settings->strategy.first = true;
            else settings->strategy.first = false;
            if(phaseTwoBox->currentIndex() == 0) settings->strategy.second = true;
            else settings->strategy.second = false;
            if(phaseThreeBox->currentIndex() == 0) settings->strategy.third = true;
            else settings->strategy.third = false;
            if(phaseFourBox->currentIndex() == 0) settings->strategy.fourth = true;
            else settings->strategy.fourth = false;
            if(phaseFiveBox->currentIndex() == 0) settings->strategy.fives = true;
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

        //hero check
        int heroIndex = -1;
        QStringList relics;
        emit getHeroRelics(tab, heroIndex, relics);
        if(heroIndex == 5) {
            const QList<QString> tempRelics = relics.toList();
            listData[fixedIndex]->controller->setHeroRelics(nullptr, &tempRelics);
        }
        else {
            listData[fixedIndex]->controller->setHeroSet(static_cast<typeSet>(heroIndex));
        }
        //

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

