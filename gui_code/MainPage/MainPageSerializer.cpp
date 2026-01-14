#include "MainPage/MainPageSerializer.h"

#include "qdebug.h"

#include "Emulators/Core/Emulator.h"
#include "Emulators/LDPlayer/ldplayer.h"
#include "User/UserProfile.h"

QByteArray MainPageSerializes::toBinary(const userProfile *user, const Emulator *emulator) {
    UserProfileBinaryData data;

    // Копируем простые поля
    data.bot_ID = user->bot_ID;
    data.user_ID = user->user_ID;
    data.history_power = user->history_power;
    data.count_units = user->count_units;
    data.state_premium = user->state_premium ? 1 : 0;
    data.state_ads = user->state_ads ? 1 : 0;
    data.subscribe = static_cast<int32_t>(user->subscribe);
    data.emulatorType = static_cast<int32_t>(user->emulatorType);

    // Копируем строки с защитой от переполнения
    strncpy(data.leftover_time,
            user->leftover_time.toUtf8().constData(),
            sizeof(data.leftover_time) - 1);
    data.leftover_time[sizeof(data.leftover_time) - 1] = '\0';

    strncpy(data.emulator_name,
            user->emulator_name.toUtf8().constData(),
            sizeof(data.emulator_name) - 1);
    data.emulator_name[sizeof(data.emulator_name) - 1] = '\0';

    if(emulator != nullptr){
        data.emulatorInstance = emulator->instance;
        strncpy(data.emulator_path,
                QString::fromStdWString(emulator->cmd).toUtf8().constData(),
                sizeof(data.emulator_path) - 1);
        data.leftover_time[sizeof(data.leftover_time) - 1] = '\0';
    } else {
        data.emulatorInstance = 0;
        data.leftover_time[sizeof(data.leftover_time) - 1] = '\0';
    }

    return QByteArray(reinterpret_cast<const char*>(&data), sizeof(data));
}

bool MainPageSerializes::fromBinary(const QByteArray &binaryData, userProfile *user, Emulator *emulator) {
    if (binaryData.size() != sizeof(UserProfileBinaryData)) {
        qWarning() << "Неверный размер данных для десериализации!"; // debug
        return false;
    }
    const UserProfileBinaryData *data = reinterpret_cast<const UserProfileBinaryData*>(binaryData.constData());

    user->bot_ID = data->bot_ID;
    user->user_ID = data->user_ID;
    user->history_power = data->history_power;
    user->count_units = data->count_units;
    user->state_premium = data->state_premium != 0;
    user->state_ads = data->state_ads != 0;
    user->subscribe = static_cast<typeSub>(data->subscribe);
    user->emulatorType = static_cast<typeEmu>(data->emulatorType);
    user->leftover_time = QString::fromUtf8(data->leftover_time);
    user->emulator_name = QString::fromUtf8(data->emulator_name);
    if(emulator == nullptr) emulator = new LDPlayer();//temp
    emulator->instance = data->emulatorInstance;
    emulator->cmd = QString::fromUtf8(data->emulator_path).toStdWString();

    return true;
}
