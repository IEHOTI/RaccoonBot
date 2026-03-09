#include "MainWindow/MainWindow.h"
#include "MyClasses/GeneralData.h"

#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QMenu>
#include <QWidgetAction>
#include <QHBoxLayout> // delete later?

#include "Lighthouse/LighthouseSettings.h"
#include "Lighthouse/Lighthouse.h"
#include "User/UserProfile.h"



void MainWindow::createLighthouseTab(QWidget *tab, int index) {
    QHBoxLayout *mainLayout = new QHBoxLayout(tab);
    mainLayout->setObjectName("lighthouseMainLayout");

    //left start
    QWidget *leftWidget = new QWidget(tab);
    leftWidget->setObjectName("lighthouseLeftWidget");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setObjectName("lighthouseLeftLayout");
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    QLabel *labelRefresh = new QLabel("Обновлять список противников", tab);
    leftLayout->addWidget(labelRefresh);

    QComboBox *algorithmBox = new QComboBox(tab);
    algorithmBox->setObjectName("algorithmBox");
    algorithmBox->addItems({"При первом поражении","Каждые 3 атаки","Каждые 5 атак"});
    leftLayout->addWidget(algorithmBox);

    QWidget *squadWidget = new QWidget(leftWidget);
    squadWidget->setObjectName("lighthouseSquadWidget");
    QHBoxLayout *squadLayout = new QHBoxLayout(squadWidget);
    squadLayout->setObjectName("lighthouseSquadLayout");
    squadLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *squadLabel = new QLabel("Отряд", squadWidget);
    squadLayout->addWidget(squadLabel);
    squadLabel->setFixedWidth(61);

    QComboBox *squadBox = new QComboBox(squadWidget);
    squadBox->setObjectName("squadBox");
    squadBox->addItems({"Не трогать", "Изменить"});
    squadLayout->addWidget(squadBox);

    QPushButton *squadButton = new QPushButton(squadWidget);
    squadButton->setObjectName("squadButton");
    squadButton->setFixedWidth(30);
    QMenu *squadMenu = new QMenu(squadWidget);
    squadMenu->setObjectName("squadMenu");

    QWidget *unitsWidget = new QWidget(squadWidget);
    unitsWidget->setObjectName("lighthouseUnitsWidget");
    createUnitsWidget(tab, unitsWidget);

    QWidgetAction *squadAction = new QWidgetAction(squadMenu);
    squadAction->setObjectName("squadAction");
    squadAction->setDefaultWidget(unitsWidget);
    squadMenu->addAction(squadAction);
    squadButton->setMenu(squadMenu);
    squadButton->setEnabled(false);

    connect(squadBox, &QComboBox::currentIndexChanged, this, [=](int index){
        squadButton->setEnabled(index != 0);
    });

    squadLayout->addWidget(squadButton);
    leftLayout->addWidget(squadWidget);

    QWidget *heroWidget = new QWidget(leftWidget);
    heroWidget->setObjectName("lighthouseHeroWidget");
    createHeroWidget(tab, heroWidget);
    heroWidget->setEnabled(true);
    leftLayout->addWidget(heroWidget);

    QWidget *petsWidget = new QWidget(leftWidget);
    petsWidget->setObjectName("lighthousePetsWidget");
    createPetsWidget(tab, petsWidget);
    petsWidget->setEnabled(true);
    leftLayout->addWidget(petsWidget);

    QWidget *titansWidget = new QWidget(leftWidget);
    titansWidget->setObjectName("lighthouseTitansWidget");
    createTitanWidget(tab, titansWidget);
    titansWidget->setEnabled(true);
    leftLayout->addWidget(titansWidget);

    leftWidget->setMinimumWidth(190);
    //left end

    //right start
    QWidget *rightWidget = new QWidget(tab);
    rightWidget->setObjectName("lighthouseRightWidget");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setObjectName("lighthouseRightLayout");
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    QWidget *checkBoxWidget = new QWidget(rightWidget);
    checkBoxWidget->setObjectName("lighthouseCheckBoxWidget");
    checkBoxWidget->setFixedHeight(80);
    QVBoxLayout *checkBoxLayout = new QVBoxLayout(checkBoxWidget);
    checkBoxLayout->setObjectName("lighthouseCheckBoxLayout");
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

    QWidget *powerWidget = new QWidget(rightWidget);
    powerWidget->setObjectName("lighthousePowerWidget");
    powerWidget->setFixedHeight(40);
    QHBoxLayout *powerLayout = new QHBoxLayout(powerWidget);
    powerLayout->setObjectName("lighthousePowerLayout");
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

    rightLayout->addWidget(powerWidget);

    QCheckBox *BLBox = new QCheckBox("BlackList", rightWidget);
    BLBox->setObjectName("BLBox");
    BLBox->setChecked(true);
    rightLayout->addWidget(BLBox);

    QCheckBox *WLBox = new QCheckBox("WhiteList", rightWidget);
    WLBox->setObjectName("WLBox");
    WLBox->setChecked(true);
    rightLayout->addWidget(WLBox);

    QLabel *labelUseDiamond = new QLabel("Использование кристаллов", tab);
    labelUseDiamond->setFixedHeight(40);
    rightLayout->addWidget(labelUseDiamond);

    QWidget *refreshWidget = new QWidget(rightWidget);
    refreshWidget->setObjectName("lighthouseRefreshWidget");
    refreshWidget->setFixedHeight(40);
    QHBoxLayout *refreshLayout = new QHBoxLayout(refreshWidget);
    refreshLayout->setObjectName("lighthouseRefreshLayout");
    refreshLayout->setContentsMargins(0, 0, 0, 0);

    QCheckBox *refreshBox = new QCheckBox("Обновление", refreshWidget);
    refreshBox->setObjectName("refreshBox");
    refreshLayout->addWidget(refreshBox);

    QLineEdit *refreshLine = new QLineEdit(refreshWidget);
    refreshLine->setObjectName("refreshLine");
    refreshLine->setFixedSize(35, 20);
    refreshLine->setEnabled(false);
    refreshLine->setText("1");
    refreshLayout->addWidget(refreshLine);

    connect(refreshBox, &QCheckBox::toggled, this, [=](bool checked) {
        refreshLine->setEnabled(checked);
    });

    QIntValidator *intValid = new QIntValidator(0, 100, refreshWidget);
    intValid->setObjectName("refreshValidator");
    refreshLine->setValidator(intValid);

    rightLayout->addWidget(refreshWidget);

    QWidget *attackWidget = new QWidget(rightWidget);
    attackWidget->setObjectName("lighthouseAttackWidget");
    attackWidget->setFixedHeight(40);
    QHBoxLayout *attackLayout = new QHBoxLayout(attackWidget);
    attackLayout->setObjectName("lighthouseAttackLayout");
    attackLayout->setContentsMargins(0, 0, 0, 0);

    QCheckBox *attackBox = new QCheckBox("Атака", attackWidget);
    attackBox->setObjectName("attackBox");
    attackLayout->addWidget(attackBox);

    QLineEdit *attackLine = new QLineEdit(attackWidget);
    attackLine->setObjectName("attackLine");
    attackLine->setFixedSize(35, 20);
    attackLine->setEnabled(false);
    attackLine->setText("1");
    attackLayout->addWidget(attackLine);

    connect(attackBox, &QCheckBox::toggled, this, [=](bool checked) {
        attackLine->setEnabled(checked);
    });
    attackLine->setValidator(intValid);

    rightLayout->addWidget(attackWidget);
    //right end

    mainLayout->addWidget(leftWidget);
    mainLayout->addWidget(rightWidget);

    int fixedIndex = index;
    connect(this,&MainWindow::getLighthouseSettings,this,[=](int local_index){
        if(local_index != fixedIndex) return;
        tab->setEnabled(false);
        userProfile *tempUser = listData[fixedIndex]->user;
        LighthouseSettings *settings = new LighthouseSettings();
        settings->history_power = tempUser->history_power;
        settings->premiumStatus = tempUser->state_premium;
        settings->watchADS = adsTask->isChecked();
        for(int i = 0; i < tempUser->count_units; i++) {
            typeSet temp;
            emit getUnitSet(tab,i,temp);
            settings->squadSet.append(temp);
            //удалить
            listErrorLogs[fixedIndex]->append("append set " + QString::number(static_cast<int>(temp)));
        }
        settings->openChest = chestTask->isChecked();
        if(refreshBox->isChecked()) settings->diamondRefresh = refreshLine->text().toInt();
        if(attackBox->isChecked()) settings->diamondAttack = attackLine->text().toInt();
        settings->black = BLBox->isChecked();
        settings->white = WLBox->isChecked();
        if(algorithmBox->currentIndex() == 0) settings->modeRefresh = -1;
        else if(algorithmBox->currentIndex() == 1) settings->modeRefresh = 3;
        else settings->modeRefresh = 5;

        //hero check
        int heroIndex = -1;
        QStringList relics;
        emit getHeroRelics(tab, heroIndex, relics);
        //

        double temp = powerLine->text().toDouble();
        if(temp > 9.9) temp = 9.9;
        else if(temp < 0.0) temp = 0.0;
        settings->rangePower = temp;
        Lighthouse *task = new Lighthouse(listData[fixedIndex]->controller);
        task->moveToThread(botThreads[fixedIndex]);
        QMetaObject::invokeMethod(task, [task, parent=listData[fixedIndex].data()](){
            task->setParent(parent);
        }, Qt::QueuedConnection);
        connect(this,&MainWindow::initLighthouse,task,&Lighthouse::Initialize,Qt::QueuedConnection);
        emit initLighthouse(settings);
        listData[fixedIndex]->listTasks.append(task);
        listData[fixedIndex]->listSettings.append(settings);

        tab->setEnabled(true);
    });
}
