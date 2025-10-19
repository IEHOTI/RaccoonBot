#include "UserProfile.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QDateTime>

userProfile::userProfile() {
    user_ID = 0;
    history_power = 0;
    count_units = 0;
    state_premium = false;
    state_ads = false;
    leftover_time = "0 d 0 h 0 m 0 s";
    emulator_name = "";
    subscribe = typeSub::admin;//UNKNOWN in RELEASE
    emulatorType = typeEmu::UNKNOWN;//UNKNOWN in RELEASE
}

userProfile::userProfile(const userProfile &other) {
    bot_ID = other.bot_ID;
    user_ID = other.user_ID;
    history_power = other.history_power;
    count_units = other.count_units;
    state_premium = other.state_premium;
    state_ads = other.state_ads;
    leftover_time = other.leftover_time;
    emulator_name = other.emulator_name;
    subscribe = other.subscribe;
    emulatorType = other.emulatorType;
}

userProfile& userProfile::operator=(const userProfile &other) {
    if (this == &other) return *this;

    bot_ID = other.bot_ID;
    user_ID = other.user_ID;
    history_power = other.history_power;
    count_units = other.count_units;
    state_premium = other.state_premium;
    state_ads = other.state_ads;
    leftover_time = other.leftover_time;
    emulator_name = other.emulator_name;
    subscribe = other.subscribe;
    emulatorType = other.emulatorType;

    return *this;
}

QString userProfile::getEmulatorType() const {
    switch (emulatorType) {
    case typeEmu::ld_player:   return "LD-Player";
    case typeEmu::nox_app:     return "NoxApp Player";
    case typeEmu::memu_app:    return "MEmu App";
    case typeEmu::mg_launcher: return "MGLauncher";
    default:                   return "Не найден эмулятор";
    }
}

QString userProfile::getSubscribeType() const {
    switch (subscribe) {
    case typeSub::Trial:     return "Пробная";
    case typeSub::Arena:     return "Арена";
    case typeSub::ArenaClan: return "Арена + Клан";
    case typeSub::Sobor:     return "Арена + Собор";
    case typeSub::SoborClan: return "Арена + Собор + Клан";
    case typeSub::Dungeon:   return "Арена + Собор + Подземелье";
    case typeSub::Full:      return "Полная";
    case typeSub::admin:     return "Ого! Вы что, админ?";
    default:                 return "Без подписки";
    }
}

void userProfile::getInfo(QWidget *widget) {
    // предполагается, что виджет уже чистый пришёл
    QHBoxLayout *generalLayout = new QHBoxLayout(widget);
    QWidget *leftInfo = new QWidget(widget);
    QWidget *rightInfo = new QWidget(widget);
    if(user_ID < 1 ){
        /////
        QVBoxLayout *leftLayout = new QVBoxLayout(leftInfo);
        QLabel *emulatorLabel = new QLabel("Эмулятор",leftInfo);
        leftLayout->addWidget(emulatorLabel);
        QLabel *emulatorLabel1 = new QLabel("Не найден",leftInfo);
        leftLayout->addWidget(emulatorLabel1);
        QLabel *accountLabel = new QLabel("Игровой ID аккаунта",leftInfo);
        leftLayout->addWidget(accountLabel);
        QLabel *accountLabel1 = new QLabel("Отсутствует",leftInfo);
        leftLayout->addWidget(accountLabel1);
        QLabel *subscribeLabel = new QLabel("Тип подписки",leftInfo);
        leftLayout->addWidget(subscribeLabel);
        QLabel *subscribeLabel1 = new QLabel("Нет подписки", leftInfo);
        leftLayout->addWidget(subscribeLabel1);
        /////

        QVBoxLayout *rightLayout = new QVBoxLayout(rightInfo);
        QLabel *HPLabel = new QLabel("Историческая мощь",rightInfo);
        rightLayout->addWidget(HPLabel);
        QLabel *HPLabel1 = new QLabel(QString::number(history_power),rightInfo);
        rightLayout->addWidget(HPLabel1);
        QLabel *countLabel = new QLabel("Максимум бойцов в казарме",rightInfo);
        rightLayout->addWidget(countLabel);
        QLabel *countLabel1 = new QLabel(QString::number(count_units),rightInfo);
        rightLayout->addWidget(countLabel1);
        QWidget *tempWidget = new QWidget(rightInfo);
        QGridLayout *premADSLayout = new QGridLayout(tempWidget);
        QLabel *premLabel = new QLabel("  Премиум  ",rightInfo);
        premADSLayout->addWidget(premLabel,0,1,Qt::AlignCenter);
        QLabel *adsLabel = new QLabel("Пропуск реклам",rightInfo);
        premADSLayout->addWidget(adsLabel,0,0,Qt::AlignCenter);
        QLabel *premCheckLabel = new QLabel("Отсутствует",rightInfo);
        premADSLayout->addWidget(premCheckLabel,1,1,Qt::AlignCenter);
        QLabel *adsCheckLabel = new QLabel("Отсутствует",rightInfo);
        premADSLayout->addWidget(adsCheckLabel,1,0,Qt::AlignCenter);
        rightLayout->addWidget(tempWidget);
    }
    else {
        /////
        QVBoxLayout *leftLayout = new QVBoxLayout(leftInfo);
        QLabel *emulatorLabel = new QLabel("Эмулятор",leftInfo);
        leftLayout->addWidget(emulatorLabel);
        QLabel *emulatorLabel1 = new QLabel(getEmulatorType() + " : " + emulator_name,leftInfo);
        leftLayout->addWidget(emulatorLabel1);
        QLabel *accountLabel = new QLabel("Игровой ID аккаунта",leftInfo);
        leftLayout->addWidget(accountLabel);
        QLabel *accountLabel1 = new QLabel(QString::number(user_ID),leftInfo);
        leftLayout->addWidget(accountLabel1);
        QLabel *subscribeLabel = new QLabel("Тип подписки",leftInfo);
        leftLayout->addWidget(subscribeLabel);
        QLabel *subscribeLabel1 = new QLabel(getSubscribeType(), leftInfo);
        leftLayout->addWidget(subscribeLabel1);
        /////

        QVBoxLayout *rightLayout = new QVBoxLayout(rightInfo);
        QLabel *HPLabel = new QLabel("Историческая мощь",rightInfo);
        rightLayout->addWidget(HPLabel);
        QLabel *HPLabel1 = new QLabel(QString::number(history_power),rightInfo);
        rightLayout->addWidget(HPLabel1);
        QLabel *countLabel = new QLabel("Максимум бойцов в казарме",rightInfo);
        rightLayout->addWidget(countLabel);
        QLabel *countLabel1 = new QLabel(QString::number(count_units),rightInfo);
        rightLayout->addWidget(countLabel1);
        QWidget *tempWidget = new QWidget(rightInfo);
        QGridLayout *premADSLayout = new QGridLayout(tempWidget);
        QLabel *premLabel = new QLabel(" Премиум ",rightInfo);
        premADSLayout->addWidget(premLabel,0,1,Qt::AlignCenter);
        QLabel *adsLabel = new QLabel("Пропуск реклам",rightInfo);
        premADSLayout->addWidget(adsLabel,0,0,Qt::AlignCenter);
        QString adsChecker, premChecker;
        if(state_premium) premChecker = "Присутствует";
        else premChecker = "Отсутствует";
        if(state_ads) adsChecker = "Присутствует";
        else adsChecker = "Отсутствует";
        QLabel *premCheckLabel = new QLabel(premChecker,rightInfo);
        premADSLayout->addWidget(premCheckLabel,1,1,Qt::AlignCenter);
        QLabel *adsCheckLabel = new QLabel(adsChecker,rightInfo);
        premADSLayout->addWidget(adsCheckLabel,1,0,Qt::AlignCenter);
        rightLayout->addWidget(tempWidget);
    }

    generalLayout->addWidget(leftInfo);
    generalLayout->addWidget(rightInfo);

    for (QObject *child : leftInfo->children()) {
        QLabel *label = qobject_cast<QLabel*>(child);
        if (label) label->setAlignment(Qt::AlignCenter);
    }
    for (QObject *child : rightInfo->children()) {
        QLabel *label = qobject_cast<QLabel*>(child);
        if (label) label->setAlignment(Qt::AlignCenter);
    }

    widget->update();
}
