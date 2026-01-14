#include "MainWindow.h"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QMenu>
#include <QStandardItemModel>
#include <QWidgetAction>

void MainWindow::createUnitsWidget(QWidget *mainTab, QWidget *widget) {
    widget->setGeometry(0,90,175,230);
    widget->setObjectName("unitWidget");

    QGridLayout *layout = new QGridLayout(widget);

    for(int i = 0; i < 8; ++i){
        QLabel *unitLabel = new QLabel(("Боец " + QString::number(i+1)),widget);
        unitLabel->setObjectName("unitLabel_" + QString::number(i+1));
        layout->addWidget(unitLabel,i,0,Qt::AlignCenter);
        QComboBox *unitSet = new QComboBox(widget);
        unitSet->setObjectName("unitSet_" + QString::number(i+1));
        ////////////////////////////к этому все привести
        QStringList options{ "Не трогать", "Набор 1", "Набор 2", "Набор 3", "Снять всё" };
        unitSet->addItems(options);
        //////////////////////////////
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
    widget->setGeometry(165,205,220,40);
    widget->setObjectName("heroWidget");

    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Реликвии",widget);
    label->setObjectName("relicsLabel");
    layout->addWidget(label);

    QPushButton *button = new QPushButton(widget);
    button->setObjectName("relicsButton");
    button->setEnabled(false);

    QComboBox *comboBox = new QComboBox(widget);
    comboBox->setObjectName("relicsComboBox");
    QStringList options = { "Не трогать", "Набор 1", "Набор 2", "Набор 3", "Снять всё", "Свой сет" };
    comboBox->addItems(options);

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
        QList<QStandardItem*> listRelics;
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_totem.png"), "Счастливый тотем мага"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/plate.png"), "Шипованный доспех"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/quiver.png"), "Заколдованный колчан"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/ring.png"), "Магический перстень"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/mage_glove.png"), "Перчатка мага"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/scope.png"), "Капюшон робина"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/shackle.png"), "Мифриловые оковы"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/shirt.png"), "Рубаха смирения"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/sleeve.png"), "Туз в рукаве"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/slingshot.png"), "Смертоносная рогатка"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/target.png"), "Точно в цель"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_shield.png"), "Стена щитов"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_glove.png"), "Перчатка воина"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_glove.png"), "Перчатка лучника"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/archer_totem.png"), "Счастливый тотем лучника"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/balls.png"), "Защита звезд"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/belt.png"), "Пояс чемпиона"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/horn.png"), "Боевой рог"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_arena.png"), "Флаг турнира"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_cw.png"), "Флаг клановой войны"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_dungeon.png"), "Флаг подземелья"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_gembay.png"), "Флаг бухты самоцветов"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/boost_portal.png"), "Флаг портала"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/chain.png"), "Дай сюда"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/clock.png"), "Часы безвременья"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_arena.png"), "Знамя света"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cathedral.png"), "Знамя доблести"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_cw.png"), "Знамя славы"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_gembay.png"), "Знамя кракена"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_lighthouse.png"), "Знамя маяка"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/flag_portal.png"), "Знамя душ"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/fountain.png"), "Фонтан юности"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/hamster.png"), "Бронированный хомяк"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/hat.png"), "Колдовская шляпа"));
        listRelics.append(new QStandardItem(QIcon(":/pages/hero_relics/warrior_totem.png"), "Счастливый тотем воина"));
        return listRelics;
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
}
