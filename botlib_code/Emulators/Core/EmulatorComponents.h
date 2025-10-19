#ifndef EMULATORCOMPONENTS_H
#define EMULATORCOMPONENTS_H
#include <string>
#include <windows.h>

class QString;

enum class typeEmu {
    ld_player,
    nox_app,
    memu_app,
    mg_launcher,
    UNKNOWN,
};

std::wstring GetCommandLineForHWND(HWND hwnd);

std::wstring sanitizePath(const std::wstring &input, typeEmu type, int &instance);

bool FindEmulator(const QString &windowName, HWND *main, HWND *game); // need bool?

#endif // EMULATORCOMPONENTS_H
