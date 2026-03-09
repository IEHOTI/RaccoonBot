#include "MainWindow.h"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QMenu>
#include <QStandardItemModel>
#include <QWidgetAction>

#include <QCoreApplication>

void MainWindow::createUnitsWidget(QWidget *mainTab, QWidget *widget) {
    //widget->setGeometry(0,90,175,230);
    widget->setObjectName("unitWidget");

    QGridLayout *layout = new QGridLayout(widget);

    for(int i = 0; i < 8; ++i){
        QLabel *unitLabel = new QLabel(("Боец " + QString::number(i+1)),widget);
        layout->addWidget(unitLabel,i,0,Qt::AlignCenter);
        QComboBox *unitSet = new QComboBox(widget);
        unitSet->setObjectName("unitSet_" + QString::number(i+1));
        unitSet->addItems({"Не трогать", "Набор 1", "Набор 2", "Набор 3", "Снять всё"});

        connect(this,&MainWindow::getUnitSet,this,[i,mainTab,unitSet](QWidget *tab, int index, typeSet &set){
            if(i != index || mainTab != tab) return;
            switch(unitSet->currentIndex()) {
            case 0:{
                set = typeSet::NOT_TOUCH;
                break;
            }
            case 1:{
                set = typeSet::SET_1;
                break;
            }
            case 2:{
                set = typeSet::SET_2;
                break;
            }
            case 3:{
                set = typeSet::SET_3;
                break;
            }
            case 4:{
                set = typeSet::UNEQUIP;
                break;
            }
            }
        });
        layout->addWidget(unitSet,i,1);
    }
}

void MainWindow::createHeroWidget(QWidget *mainTab, QWidget *widget){
    //widget->setGeometry(165,205,220,40);
    widget->setObjectName("heroWidget");

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    QLabel *label = new QLabel("Реликвии",widget);
    layout->addWidget(label);

    QPushButton *button = new QPushButton(widget);
    button->setFixedWidth(30);
    button->setObjectName("relicsButton");
    button->setEnabled(false);

    QComboBox *comboBox = new QComboBox(widget);
    comboBox->setObjectName("relicsComboBox");
    comboBox->addItems({"Не трогать", "Набор 1", "Набор 2", "Набор 3", "Снять всё", "Свой сет"});

    layout->addWidget(comboBox);
    layout->addWidget(button);
    widget->setLayout(layout);

    connect(comboBox,&QComboBox::currentIndexChanged,this,[=](){
        if(comboBox->currentIndex() == 5) button->setEnabled(true);
        else button->setEnabled(false);
    });

    QMenu *setMenu = new QMenu(widget);
    setMenu->setObjectName("relicsMenu");
    QWidget *setWidget = new QWidget();
    setWidget->setObjectName("relicsSetWidget");
    QGridLayout *setLayout = new QGridLayout(setWidget);

    QComboBox *setMainRelicBox = new QComboBox(setWidget);
    setMainRelicBox->setObjectName("mainRelicComboBox");
    QStandardItemModel *modelMainRelic = new QStandardItemModel(setMainRelicBox);

    QComboBox *setFirstSubRelic = new QComboBox(setWidget);
    setFirstSubRelic->setObjectName("firstSubRelicComboBox");
    QStandardItemModel *modelFirstSubRelic = new QStandardItemModel(setFirstSubRelic);
    QComboBox *setSecondSubRelic = new QComboBox(setWidget);
    setSecondSubRelic->setObjectName("secondSubRelicComboBox");
    QStandardItemModel *modelSecondSubRelic = new QStandardItemModel(setSecondSubRelic);
    QComboBox *setThirdSubRelic = new QComboBox(setWidget);
    setThirdSubRelic->setObjectName("thirdSubRelicComboBox");
    QStandardItemModel *modelThirdSubRelic = new QStandardItemModel(setThirdSubRelic);
    QComboBox *setFourthSubRelic = new QComboBox(setWidget);
    setFourthSubRelic->setObjectName("fourthSubRelicComboBox");
    QStandardItemModel *modelFourthSubRelic = new QStandardItemModel(setFourthSubRelic);

    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/acid.png"), "Банка с кислотой"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/anger_totem.png"), "Огненный тотем"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/ankh.png"), "Анкх"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/bomb.png"), "Трупобум"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/book.png"), "Книга ветра"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/equilibrium.png"), "Символ равновесия"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/glaive.png"), "Яростная глефа"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/head.png"), "Голова ужаса"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/heart.png"), "Терновое сердце"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/helmet.png"), "Рогатый шлем"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/lamp.png"), "Волшебная лампа"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/mask.png"), "Маска безмолвия"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/medkit.png"), "Медпакет"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/mjolnir.png"), "Хасльёльнир"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/shield.png"), "Полумагический щит"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/sunbeam.png"), "Солнечный зайчик"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/tomb.png"), "Очень тяжелая плита"));
    modelMainRelic->appendRow(new QStandardItem(QIcon(":/pages/hero_relics/watch.png"), "Временный парадокс"));

    ////
    auto makeRelicsColumn = []() {
        QList<QStandardItem*> listTitans;
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_totem.png"), "Счастливый тотем мага"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/plate.png"), "Шипованный доспех"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/quiver.png"), "Заколдованный колчан"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/ring.png"), "Магический перстень"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_glove.png"), "Перчатка мага"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/scope.png"), "Капюшон робина"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/shackle.png"), "Мифриловые оковы"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/shirt.png"), "Рубаха смирения"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/sleeve.png"), "Туз в рукаве"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/slingshot.png"), "Смертоносная рогатка"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/target.png"), "Точно в цель"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_shield.png"), "Стена щитов"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_glove.png"), "Перчатка воина"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_glove.png"), "Перчатка лучника"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_totem.png"), "Счастливый тотем лучника"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/balls.png"), "Защита звезд"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/belt.png"), "Пояс чемпиона"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/horn.png"), "Боевой рог"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_arena.png"), "Флаг турнира"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_cw.png"), "Флаг клановой войны"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_dungeon.png"), "Флаг подземелья"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_gembay.png"), "Флаг бухты самоцветов"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_portal.png"), "Флаг портала"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/chain.png"), "Дай сюда"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/clock.png"), "Часы безвременья"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_arena.png"), "Знамя света"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cathedral.png"), "Знамя доблести"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cw.png"), "Знамя славы"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_gembay.png"), "Знамя кракена"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_lighthouse.png"), "Знамя маяка"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_portal.png"), "Знамя душ"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/fountain.png"), "Фонтан юности"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/hamster.png"), "Бронированный хомяк"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/hat.png"), "Колдовская шляпа"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_totem.png"), "Счастливый тотем воина"));
        return listTitans;
    };

    modelFirstSubRelic->appendColumn(makeRelicsColumn());
    modelSecondSubRelic->appendColumn(makeRelicsColumn());
    modelThirdSubRelic->appendColumn(makeRelicsColumn());
    modelFourthSubRelic->appendColumn(makeRelicsColumn());

    setMainRelicBox->setModel(modelMainRelic);
    setFirstSubRelic->setModel(modelFirstSubRelic);
    setSecondSubRelic->setModel(modelSecondSubRelic);
    setThirdSubRelic->setModel(modelThirdSubRelic);
    setFourthSubRelic->setModel(modelFourthSubRelic);

    setLayout->addWidget(new QLabel("Основная реликвия"),0,0);
    setLayout->addWidget(setMainRelicBox,0,1);
    setLayout->addWidget(new QLabel("Дополнительные рекликвии"),1,0);
    setLayout->addWidget(setFirstSubRelic,1,1);
    setLayout->addWidget(setSecondSubRelic,2,1);
    setLayout->addWidget(setThirdSubRelic,3,1);
    setLayout->addWidget(setFourthSubRelic,4,1);

    setWidget->setLayout(setLayout);
    QWidgetAction *setAlgorithmWidget = new QWidgetAction(setMenu);
    setAlgorithmWidget->setDefaultWidget(setWidget);
    setMenu->addAction(setAlgorithmWidget);

    button->setMenu(setMenu);

    connect(this,&MainWindow::getHeroRelics,this,[=](QWidget *tab, int &index, QStringList &relics){
        if(mainTab != tab) return;
        index = comboBox->currentIndex();
        if (index == 5) {
            if (!relics.contains(setMainRelicBox->currentText())) relics.append(setMainRelicBox->currentText());
            if (!relics.contains(setFirstSubRelic->currentText())) relics.append(setFirstSubRelic->currentText());
            if (!relics.contains(setSecondSubRelic->currentText())) relics.append(setSecondSubRelic->currentText());
            if (!relics.contains(setThirdSubRelic->currentText())) relics.append(setThirdSubRelic->currentText());
            if (!relics.contains(setFourthSubRelic->currentText())) relics.append(setFourthSubRelic->currentText());
        } else relics.clear();
    });
    widget->update();
    QCoreApplication::processEvents();
}

void MainWindow::createTitanWidget(QWidget *mainTab, QWidget *widget) {
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    QLabel *label = new QLabel("Титаны",widget);
    layout->addWidget(label);


    QPushButton *button = new QPushButton(widget);
    button->setFixedWidth(30);
    button->setEnabled(false);

    QComboBox *comboBox = new QComboBox(widget);
    comboBox->addItems({"Не трогать", "Изменить"});

    layout->addWidget(comboBox);
    layout->addWidget(button);
    widget->setLayout(layout);

    connect(comboBox,&QComboBox::currentIndexChanged,this,[=](){
        if(comboBox->currentIndex() != 0) button->setEnabled(true);
        else button->setEnabled(false);
    });

    QMenu *setMenu = new QMenu(widget);
    QWidget *setWidget = new QWidget();
    QGridLayout *setLayout = new QGridLayout(setWidget);

    QComboBox *setFirstTitanBox = new QComboBox(setWidget);
    QStandardItemModel *modelFirstTitanBox = new QStandardItemModel(setFirstTitanBox);
    QComboBox *setSecondTitanBox = new QComboBox(setWidget);
    QStandardItemModel *modelSecondTitanBox = new QStandardItemModel(setSecondTitanBox);
    QComboBox *setThirdTitanBox = new QComboBox(setWidget);
    QStandardItemModel *modelThirdTitanBox  = new QStandardItemModel(setThirdTitanBox);

    auto makeTitanColumn = []() {
        QList<QStandardItem*> listTitans;
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_totem.png"), "Счастливый тотем мага"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/plate.png"), "Шипованный доспех"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/quiver.png"), "Заколдованный колчан"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/ring.png"), "Магический перстень"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_glove.png"), "Перчатка мага"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/scope.png"), "Капюшон робина"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/shackle.png"), "Мифриловые оковы"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/shirt.png"), "Рубаха смирения"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/sleeve.png"), "Туз в рукаве"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/slingshot.png"), "Смертоносная рогатка"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/target.png"), "Точно в цель"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_shield.png"), "Стена щитов"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_glove.png"), "Перчатка воина"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_glove.png"), "Перчатка лучника"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_totem.png"), "Счастливый тотем лучника"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/balls.png"), "Защита звезд"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/belt.png"), "Пояс чемпиона"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/horn.png"), "Боевой рог"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_arena.png"), "Флаг турнира"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_cw.png"), "Флаг клановой войны"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_dungeon.png"), "Флаг подземелья"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_gembay.png"), "Флаг бухты самоцветов"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_portal.png"), "Флаг портала"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/chain.png"), "Дай сюда"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/clock.png"), "Часы безвременья"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_arena.png"), "Знамя света"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cathedral.png"), "Знамя доблести"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cw.png"), "Знамя славы"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_gembay.png"), "Знамя кракена"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_lighthouse.png"), "Знамя маяка"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_portal.png"), "Знамя душ"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/fountain.png"), "Фонтан юности"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/hamster.png"), "Бронированный хомяк"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/hat.png"), "Колдовская шляпа"));
        listTitans.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_totem.png"), "Счастливый тотем воина"));
        return listTitans;
    };
    modelFirstTitanBox->appendColumn(makeTitanColumn());
    modelSecondTitanBox->appendColumn(makeTitanColumn());
    modelThirdTitanBox->appendColumn(makeTitanColumn());

    setFirstTitanBox->setModel(modelFirstTitanBox);
    setSecondTitanBox->setModel(modelSecondTitanBox);
    setThirdTitanBox->setModel(modelThirdTitanBox);

    QComboBox *setFirstTitanSet = new QComboBox(setWidget);
    setFirstTitanSet->addItems({"Не трогать стигмы", "Набор 1", "Набор 2", "Набор 3", "Снять всё"});
    QComboBox *setSecondTitanSet = new QComboBox(setWidget);
    setSecondTitanSet->addItems({"Не трогать стигмы", "Набор 1", "Набор 2", "Набор 3", "Снять всё"});
    QComboBox *setThirdTitanSet = new QComboBox(setWidget);
    setThirdTitanSet->addItems({"Не трогать стигмы", "Набор 1", "Набор 2", "Набор 3", "Снять всё"});

    setLayout->addWidget(new QLabel("Первый титан"),0,0,Qt::AlignCenter);
    setLayout->addWidget(new QLabel("Второй титан"),0,1,Qt::AlignCenter);
    setLayout->addWidget(new QLabel("Третий титан"),0,2,Qt::AlignCenter);
    setLayout->addWidget(setFirstTitanBox,1,0);
    setLayout->addWidget(setSecondTitanBox,1,1);
    setLayout->addWidget(setThirdTitanBox,1,2);
    setLayout->addWidget(setFirstTitanSet,2,0);
    setLayout->addWidget(setSecondTitanSet,2,1);
    setLayout->addWidget(setThirdTitanSet,2,2);

    setWidget->setLayout(setLayout);
    QWidgetAction *setAlgorithmWidget = new QWidgetAction(setMenu);
    setAlgorithmWidget->setDefaultWidget(setWidget);
    setMenu->addAction(setAlgorithmWidget);

    button->setMenu(setMenu);
    //connect later
}

void MainWindow::createPetsWidget(QWidget *mainTab, QWidget *widget) {
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    QLabel *label = new QLabel("Питомцы",widget);
    layout->addWidget(label);

    QComboBox *comboBox = new QComboBox(widget);
    comboBox->addItems({"Не трогать", "Изменить"});
    layout->addWidget(comboBox);

    QPushButton *button = new QPushButton(widget);
    button->setFixedWidth(30);
    button->setEnabled(false);
    layout->addWidget(button);

    widget->setLayout(layout);

    connect(comboBox,&QComboBox::currentIndexChanged,this,[=](){
        if(comboBox->currentIndex() != 0) button->setEnabled(true);
        else button->setEnabled(false);
    });

    QMenu *setMenu = new QMenu(widget);
    QWidget *setWidget = new QWidget();
    QGridLayout *setLayout = new QGridLayout(setWidget);

    QComboBox *setFirstPetsBox = new QComboBox(setWidget);
    QStandardItemModel *modelFirstPetsBox = new QStandardItemModel(setFirstPetsBox);
    QComboBox *setSecondPetsBox = new QComboBox(setWidget);
    QStandardItemModel *modelSecondPetsBox = new QStandardItemModel(setSecondPetsBox);
    QComboBox *setThirdPetsBox = new QComboBox(setWidget);
    QStandardItemModel *modelThirdPetsBox  = new QStandardItemModel(setThirdPetsBox);

    auto makePetsColumn = []() {
        QList<QStandardItem*> listPets;
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_totem.png"), "Счастливый тотем мага"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/plate.png"), "Шипованный доспех"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/quiver.png"), "Заколдованный колчан"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/ring.png"), "Магический перстень"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_glove.png"), "Перчатка мага"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/scope.png"), "Капюшон робина"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/shackle.png"), "Мифриловые оковы"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/shirt.png"), "Рубаха смирения"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/sleeve.png"), "Туз в рукаве"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/slingshot.png"), "Смертоносная рогатка"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/target.png"), "Точно в цель"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_shield.png"), "Стена щитов"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_glove.png"), "Перчатка воина"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_glove.png"), "Перчатка лучника"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_totem.png"), "Счастливый тотем лучника"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/balls.png"), "Защита звезд"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/belt.png"), "Пояс чемпиона"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/horn.png"), "Боевой рог"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_arena.png"), "Флаг турнира"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_cw.png"), "Флаг клановой войны"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_dungeon.png"), "Флаг подземелья"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_gembay.png"), "Флаг бухты самоцветов"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_portal.png"), "Флаг портала"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/chain.png"), "Дай сюда"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/clock.png"), "Часы безвременья"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_arena.png"), "Знамя света"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cathedral.png"), "Знамя доблести"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cw.png"), "Знамя славы"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_gembay.png"), "Знамя кракена"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_lighthouse.png"), "Знамя маяка"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_portal.png"), "Знамя душ"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/fountain.png"), "Фонтан юности"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/hamster.png"), "Бронированный хомяк"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/hat.png"), "Колдовская шляпа"));
        listPets.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_totem.png"), "Счастливый тотем воина"));
        return listPets;
    };
    modelFirstPetsBox->appendColumn(makePetsColumn());
    modelSecondPetsBox->appendColumn(makePetsColumn());
    modelThirdPetsBox->appendColumn(makePetsColumn());

    setFirstPetsBox->setModel(modelFirstPetsBox);
    setSecondPetsBox->setModel(modelSecondPetsBox);
    setThirdPetsBox->setModel(modelThirdPetsBox);

    setLayout->addWidget(new QLabel("Первый питомец"),0,0,Qt::AlignCenter);
    setLayout->addWidget(new QLabel("Второй питомец"),0,1,Qt::AlignCenter);
    setLayout->addWidget(new QLabel("Третий питомец"),0,2,Qt::AlignCenter);
    setLayout->addWidget(setFirstPetsBox,1,0);
    setLayout->addWidget(setSecondPetsBox,1,1);
    setLayout->addWidget(setThirdPetsBox,1,2);

    setWidget->setLayout(setLayout);
    QWidgetAction *setAlgorithmWidget = new QWidgetAction(setMenu);
    setAlgorithmWidget->setDefaultWidget(setWidget);
    setMenu->addAction(setAlgorithmWidget);

    button->setMenu(setMenu);
    //connect later
}
