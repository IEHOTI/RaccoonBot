#include "mainwindow.h"
void MainWindow::createCathedralTab(QWidget *tab,int index) {
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

    QRadioButton* fullButton = new QRadioButton("Полное прохождение",tab);
    fullButton->setGeometry(190,25,185,20);
    fullButton->setChecked(true);

    QRadioButton* fastButton = new QRadioButton("Боссы и миньоны",tab);
    fastButton->setGeometry(190,50,185,20);

    QButtonGroup* mode = new QButtonGroup(tab);
    mode->addButton(fullButton);
    mode->addButton(fastButton);

    QLabel* modeLabel = new QLabel("Сложность",tab);
    modeLabel->setGeometry(190,75,90,20);

    QComboBox* modeBox = new QComboBox(tab);
    modeBox->setGeometry(275,75,100,20);
    modeBox->addItem("Нормальная");
    modeBox->addItem("Сложная");
    modeBox->addItem("Кошмарная");

    QLabel* resourceLabel = new QLabel("Расходовать",tab);
    resourceLabel->setGeometry(190,100,90,20);

    QComboBox* resourceBox = new QComboBox(tab);
    resourceBox->setGeometry(275,100,100,20);
    resourceBox->addItem("Яблоки");
    resourceBox->addItem("Ключи");
    resourceBox->addItem("Яблоки > Ключи");
    resourceBox->addItem("Ключи > Яблоки");

    QLabel* countLabel = new QLabel("Количество походов",tab);
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

    connect(this,&MainWindow::getCathedralSettings,this,[=](){
        userProfile *tempUser = listData[index]->user;
        CathedralSettings* settings = new CathedralSettings();
        settings->history_power = tempUser->history_power;
        QString str = countBox->currentText();
        if (str == "∞") settings->count = -1;
        else settings->count = str.toInt();
        settings->premiumStatus = tempUser->state_premium;
        if(adsTask->isChecked()) settings->watchADS = true;
        else settings->watchADS = false;
        //
        settings->modeDifficult = modeBox->currentIndex();
        if(bestButton->isChecked()) settings->modeSquad = 0;
        if(lastButton->isChecked()) settings->modeSquad = 1;
        if(barrackButton->isChecked()) settings->modeSquad = 2;
        settings->modeKey = resourceBox->currentIndex();
        if(fullButton->isChecked()) settings->fullGamePass = true;
        else settings->fullGamePass = false;
        if(chestTask->isChecked()) settings->openChest = true;
        else settings->openChest = false;
        int temp = foodLine->text().remove(' ').toInt();
        if(temp > 2000000) temp = 2000000;
        else if(temp < 0) temp = 0;
        settings->saveApple = temp;
        Cathedral *task = new Cathedral(listData[index]->controller);
        task->moveToThread(botThreads[index]);
        QMetaObject::invokeMethod(task, [task, parent=listData[index].data()](){
            task->setParent(parent);
        }, Qt::QueuedConnection);
        connect(this,&MainWindow::initCathedral,task,&Cathedral::Initialize,Qt::QueuedConnection);
        emit initCathedral(settings);
        listData[index]->listTasks.append(task);
        listData[index]->listSettings.append(settings);

        tab->setEnabled(true);//?
    });
}
