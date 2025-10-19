#include "mainwindow.h"

#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>

#include "Lighthouse/LighthouseSettings.h"
#include "Lighthouse/Lighthouse.h"
#include "User/UserProfile.h"

#include "generaldata.h"

void MainWindow::createLighthouseTab(QWidget *tab, int index) {
    QCheckBox* adsTask = new QCheckBox("Смотреть рекламу",tab);
    adsTask->setGeometry(5,0,175,20);

    QCheckBox* chestTask = new QCheckBox("Автооткрытие сундуков",tab);
    chestTask->setGeometry(175,0,185,20);

    QLabel *labelRefresh = new QLabel("Частота обновлений",tab);
    labelRefresh->setGeometry(15,25,150,20);

    QComboBox *algorithmBox = new QComboBox(tab);
    algorithmBox->setGeometry(5,60,160,20);
    algorithmBox->addItem("При первом поражении");
    algorithmBox->addItem("Каждые 3 удара");
    algorithmBox->addItem("Каждые 5 ударов");

    QLabel *labelUseDiamond = new QLabel("Использование кристаллов",tab);
    labelUseDiamond->setGeometry(175,45,185,20);

    QCheckBox *useRefresh = new QCheckBox("Обновление",tab);
    useRefresh->setGeometry(175,75,140,20);

    QLineEdit *lineRefresh = new QLineEdit(tab);
    lineRefresh->setGeometry(340,75,35,20);
    lineRefresh->setEnabled(false);
    lineRefresh->setText("1");

    connect(useRefresh,&QCheckBox::toggled,this,[=](bool checked) {
        lineRefresh->setEnabled(checked);
    });

    QIntValidator *validRefresh = new QIntValidator(0,100,tab);
    lineRefresh->setValidator(validRefresh);

    QCheckBox *useAttack = new QCheckBox("Атака",tab);
    useAttack->setGeometry(175,105,140,20);

    QLineEdit *lineAttack = new QLineEdit(tab);
    lineAttack->setGeometry(340,105,35,20);
    lineAttack->setEnabled(false);
    lineAttack->setText("1");

    connect(useAttack,&QCheckBox::toggled,this,[=](bool checked) {
        lineAttack->setEnabled(checked);
    });
    lineAttack->setValidator(validRefresh);

    QLabel* powerLabel = new QLabel("Множитель мощи",tab);
    powerLabel->setGeometry(175,145,140,20);

    QLineEdit* powerLine = new QLineEdit(tab);
    powerLine->setGeometry(340,145,35,20);
    powerLine->setText("1.0");

    QDoubleValidator* powerValid = new QDoubleValidator(0.0,9.9,1,tab);
    powerValid->setNotation(QDoubleValidator::StandardNotation);
    powerValid->setLocale(QLocale::C);
    powerLine->setValidator(powerValid);

    QCheckBox *BLBox = new QCheckBox("BlackList",tab);
    BLBox->setGeometry(175,175,90,20);
    BLBox->setChecked(true);

    QCheckBox *WLBox = new QCheckBox("WhiteList",tab);
    WLBox->setGeometry(285,175,90,20);
    WLBox->setChecked(true);

    ////////////
    QWidget *unitFirstWidget = new QWidget(tab);
    createUnitsWidget(tab,unitFirstWidget);
    unitFirstWidget->setEnabled(true);

    QWidget *heroWidget = new QWidget(tab);
    createHeroWidget(tab, heroWidget);
    heroWidget->setEnabled(true);
    ///////

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
        if(useRefresh->isChecked()) settings->diamondRefresh = lineRefresh->text().toInt();
        if(useAttack->isChecked()) settings->diamondAttack = lineAttack->text().toInt();
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
