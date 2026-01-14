#ifndef MAINPAGESERIALIZER_H
#define MAINPAGESERIALIZER_H

#include "qstringview.h"
#include <cstdint>
#include <cstring>

struct userProfile;
class Emulator;

#pragma pack(push, 1)  // Упаковка структуры для бинарной сериализации
struct UserProfileBinaryData {
    // Все поля должны быть POD типами!
    int32_t bot_ID;           // 4 байта
    int32_t user_ID;          // 4 байта
    int32_t history_power;    // 4 байта
    int32_t count_units;      // 4 байта
    int8_t  state_premium;    // 1 байт
    int8_t  state_ads;        // 1 байт
    int32_t subscribe;        // 4 байта
    int32_t emulatorType;     // 4 байта
    int32_t emulatorInstance; // 4 байта

    // Строки фиксированной длины
    char leftover_time[32];   // "HH:MM:SS" или timestamp
    char emulator_name[512];  // Имя эмулятора
    char emulator_path[3096]; // путь эмулятора

    UserProfileBinaryData() {
        memset(this, 0, sizeof(UserProfileBinaryData));
    }
};
#pragma pack(pop)

static_assert(sizeof(UserProfileBinaryData) == 3670,
              "UserProfileBinaryData должен быть ровно 3670 байт");


namespace MainPageSerializes {
QByteArray toBinary(const userProfile *user, const Emulator *emulator);
bool fromBinary(const QByteArray &binaryData, userProfile *user, Emulator *emulator);
}
#endif // MAINPAGESERIALIZER_H
