#include "MainWindow/MainWindow.h"
#include "MyClasses/GeneralData.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QWidgetAction>

#include "Cathedral/CathedralSettings.h"
#include "Cathedral/Cathedral.h"
#include "User/UserProfile.h"

void MainWindow::createCathedralTab(QWidget *tab,int index) {
    QHBoxLayout *mainLayout = new QHBoxLayout(tab);
    mainLayout->setObjectName("cathedralMainLayout");

    //left start
    QWidget *leftWidget = new QWidget(tab);
    leftWidget->setObjectName("cathedralLeftWidget");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setObjectName("cathedralLeftLayout");
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    QWidget *difficultWidget = new QWidget(leftWidget);
    difficultWidget->setObjectName("difficultWidget");
    difficultWidget->setFixedHeight(40);
    QHBoxLayout *difficultLayout = new QHBoxLayout(difficultWidget);
    difficultLayout->setObjectName("difficultLayout");
    difficultLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *difficultLabel = new QLabel("Сложность", tab);
    difficultLayout->addWidget(difficultLabel);

    QComboBox *difficultBox = new QComboBox(tab);
    difficultBox->setObjectName("difficultBox");
    difficultBox->addItems({"Нормальная","Сложная","Кошмарная"});
    difficultLayout->addWidget(difficultBox);

    leftLayout->addWidget(difficultWidget);

    QWidget *squadWidget = new QWidget(leftWidget);
    squadWidget->setObjectName("cathedralSquadWidget");
    QVBoxLayout *squadLayout = new QVBoxLayout(squadWidget);
    squadLayout->setObjectName("cathedralSquadLayout");
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
    barrackWidget->setObjectName("cathedralBarrackWidget");
    QHBoxLayout *barrackLayout = new QHBoxLayout(barrackWidget);
    barrackLayout->setObjectName("cathedralBarrackLayout");
    barrackLayout->setContentsMargins(0, 0, 0, 0);

    QRadioButton *barrackButton = new QRadioButton("Отряд с казармы", barrackWidget);
    barrackButton->setObjectName("barrackButton");
    barrackLayout->addWidget(barrackButton);

    QPushButton *unitsButton = new QPushButton(barrackWidget);
    unitsButton->setObjectName("cathedralUnitsButton");
    unitsButton->setFixedWidth(30);
    barrackLayout->addWidget(unitsButton);

    QMenu *unitsMenu = new QMenu(barrackWidget);
    unitsMenu->setObjectName("cathedralUnitsMenu");

    QWidget *unitsWidget = new QWidget(barrackWidget);
    unitsWidget->setObjectName("cathedralUnitsWidget");
    createUnitsWidget(tab, unitsWidget);

    QWidgetAction *unitsAction = new QWidgetAction(unitsMenu);
    unitsAction->setObjectName("cathedralUnitsAction");
    unitsAction->setDefaultWidget(unitsWidget);
    unitsMenu->addAction(unitsAction);
    unitsButton->setMenu(unitsMenu);
    unitsButton->setEnabled(false);

    connect(barrackButton, &QRadioButton::toggled, this, [=](bool checked){
        unitsButton->setEnabled(checked);
    });

    QButtonGroup *squadGroup = new QButtonGroup(squadWidget);
    squadGroup->setObjectName("squadGroup");
    squadGroup->addButton(bestButton);
    squadGroup->addButton(lastButton);
    squadGroup->addButton(barrackButton);

    squadLayout->addWidget(barrackWidget);
    leftLayout->addWidget(squadWidget);

    QWidget *additionWidget = new QWidget(leftWidget);
    additionWidget->setObjectName("cathedralAdditionWidget");
    QVBoxLayout *additionLayout = new QVBoxLayout(additionWidget);
    additionLayout->setObjectName("cathedralAdditionLayout");
    additionLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *heroWidget = new QWidget(additionWidget);
    heroWidget->setObjectName("cathedralHeroWidget");
    createHeroWidget(tab, heroWidget);
    heroWidget->setEnabled(true);
    additionLayout->addWidget(heroWidget);

    QWidget *petsWidget = new QWidget(additionWidget);
    petsWidget->setObjectName("cathedralPetsWidget");
    createPetsWidget(tab, petsWidget);
    petsWidget->setEnabled(true);
    additionLayout->addWidget(petsWidget);

    QWidget *titansWidget = new QWidget(additionWidget);
    titansWidget->setObjectName("cathedralTitansWidget");
    createTitanWidget(tab, titansWidget);
    titansWidget->setEnabled(true);
    additionLayout->addWidget(titansWidget);

    leftLayout->addWidget(additionWidget);
    leftWidget->setMinimumWidth(190);
    //left end

    //right start
    QWidget *rightWidget = new QWidget(tab);
    rightWidget->setObjectName("cathedralRightWidget");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setObjectName("cathedralRightLayout");
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QWidget *checkBoxWidget = new QWidget(rightWidget);
    checkBoxWidget->setObjectName("cathedralCheckBoxWidget");
    checkBoxWidget->setFixedHeight(80);
    QVBoxLayout *checkBoxLayout = new QVBoxLayout(checkBoxWidget);
    checkBoxLayout->setObjectName("cathedralCheckBoxLayout");
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

    rightLayout->addWidget(checkBoxWidget);

    QWidget *passingWidget = new QWidget(rightWidget);
    passingWidget->setObjectName("passingWidget");
    QVBoxLayout *passingLayout = new QVBoxLayout(passingWidget);
    passingLayout->setObjectName("passingLayout");
    passingLayout->setContentsMargins(0, 0, 0, 0);

    QRadioButton *fullButton = new QRadioButton("Полное прохождение", passingWidget);
    fullButton->setObjectName("fullButton");
    fullButton->setChecked(true);
    passingLayout->addWidget(fullButton);

    QRadioButton *fastButton = new QRadioButton("Боссы и миньоны", passingWidget);
    fastButton->setObjectName("fastButton");
    passingLayout->addWidget(fastButton);

    QButtonGroup *modeGroup = new QButtonGroup(passingWidget);
    modeGroup->setObjectName("modeGroup");
    modeGroup->addButton(fullButton);
    modeGroup->addButton(fastButton);

    rightLayout->addWidget(passingWidget);

    QWidget *consumeWidget = new QWidget(rightWidget);
    consumeWidget->setObjectName("cathedralConsumeWidget");
    consumeWidget->setFixedHeight(60);
    QVBoxLayout *consumeLayout = new QVBoxLayout(consumeWidget);
    consumeLayout->setObjectName("cathedralConsumeLayout");
    consumeLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *consumeLabel = new QLabel("Используемые расходники", consumeWidget);
    consumeLayout->addWidget(consumeLabel);

    QComboBox *consumeBox = new QComboBox(consumeWidget);
    consumeBox->setObjectName("consumeBox");
    consumeBox->addItems({"Яблоки","Ключи","Яблоки > Ключи", "Ключи > Яблоки"});
    consumeLayout->addWidget(consumeBox);

    rightLayout->addWidget(consumeWidget);

    QWidget *saverWidget = new QWidget(rightWidget);
    saverWidget->setObjectName("cathedralSaverWidget");
    saverWidget->setFixedHeight(40);
    QHBoxLayout *saverLayout = new QHBoxLayout(saverWidget);
    saverLayout->setObjectName("cathedralSaverLayout");
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
    counterWidget->setObjectName("cathedralCounterWidget");
    counterWidget->setFixedHeight(40);
    QHBoxLayout *counterLayout = new QHBoxLayout(counterWidget);
    counterLayout->setObjectName("cathedralCounterLayout");
    counterLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *counterLabel = new QLabel("Количество походов", counterWidget);
    counterLayout->addWidget(counterLabel);

    QComboBox *counterBox = new QComboBox(counterWidget);
    counterBox->setObjectName("counterBox");
    counterBox->setFixedSize(40, 20);
    counterBox->addItems({"1","2","3","4","5","7","10","20","30","∞"});
    counterLayout->addWidget(counterBox);

    rightLayout->addWidget(counterWidget);
    //right end

    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(rightWidget);

    int fixedIndex = index;
    connect(this,&MainWindow::getCathedralSettings,this,[=](int local_index){
        if(local_index != fixedIndex) return;
        tab->setEnabled(false);
        userProfile *tempUser = listData[fixedIndex]->user;
        CathedralSettings *settings = new CathedralSettings();
        settings->history_power = tempUser->history_power;
        QString str = counterBox->currentText();
        if (str == "∞") settings->count = -1;
        else settings->count = str.toInt();
        settings->premiumStatus = tempUser->state_premium;
        settings->watchADS = adsTask->isChecked();
        //
        settings->modeDifficult = difficultBox->currentIndex();
        if(bestButton->isChecked()) settings->modeSquad = 0;
        if(lastButton->isChecked()) settings->modeSquad = 1;
        if(barrackButton->isChecked()) settings->modeSquad = 2;
        settings->modeKey = consumeBox->currentIndex();
        settings->fullGamePass = fullButton->isChecked();
        settings->openChest = chestTask->isChecked();
        int temp = saverLine->text().remove(' ').toInt();
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
