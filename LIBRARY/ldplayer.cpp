#include "ldplayer.h"

LDPlayer::LDPlayer(QObject *parent) : Emulator(parent) {cmd = L""; instance = 0;}

void LDPlayer::Start(bool *start){
    emit Logging("Путь эмулятора: " + QString::fromStdWString(cmd));
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    emit Logging("Запускаю эмулятор");
    if (!CreateProcessW(NULL, LPWSTR(cmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        emit Logging("Не удалось запустить эмулятор.");// мб потом более подробный лог можно будет сделать
        *start = false;
        return;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    *start = true;
}

void LDPlayer::Stop(HWND *main) {
    emit Logging("Закрываю эмулятор.");
    if(*main != NULL) {
        PostMessage(*main,WM_CLOSE,0,0);//закрыть эмулятор
        *main = 0;
    }
    emit Logging("Эмулятор закрыт.");
}

void LDPlayer::Initialize(HWND *main) {
    char name[256];
    emit Logging("Инициализирую эмулятор");
    typeEmu type = typeEmu::UNKNOWN;
    GetClassNameA(*main, name, sizeof(name));
    if (strcmp(name, "LDPlayerMainFrame") == 0) type = typeEmu::ld_player;
    name[0]= '\0';
    GetWindowTextA(*main, name, sizeof(name));
    cmd.clear();
    cmd = sanitizePath(GetCommandLineForHWND(*main),type,instance);
    if(type == typeEmu::ld_player) cmd += L" launchex --index " + to_wstring(instance) + L" --packagename com.my.hc.rpg.kingdom.simulator"; // - why "-"?
    else if(type == typeEmu::memu_app) cmd += L" applink com.my.hc.rpg.kingdom.simulator";//??
}

LDPlayer &LDPlayer::operator =(const Emulator &other) {
    if (this == &other) return *this;

    QObject::setParent(other.parent());
    cmd = static_cast<const LDPlayer&>(other).cmd;
    instance = static_cast<const LDPlayer&>(other).instance;

    return *this;
}

void LDPlayer::FixSize(HWND *main, HWND *game) {
    SetWindowPos(*main, HWND_BOTTOM, 1, 1, 900, 600+34, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    Sleep(300);
    if (!SetWindowPos(*game, HWND_BOTTOM, 0, 34, 900, 600, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) {
        emit Logging("Не удалось починить размер эмулятора. Попробуйте кнопку \"Починить всё\"");
        return;
    }
    Sleep(300);
}

void LDPlayer::FixPos(HWND *main, HWND *game) {
    SetWindowPos(*main, HWND_BOTTOM, 1, 1, 900, 600+34, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    Sleep(300);
    if (!SetWindowPos(*game, HWND_BOTTOM, 0, 34, 900, 600, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) {
        emit Logging("Не удалось переместить окно эмулятора. Попробуйте кнопку \"Починить всё\"");
        return;
    }
    Sleep(300);
}

void LDPlayer::FixAll(HWND *main, HWND *game) {
    LONG_PTR style = GetWindowLongPtr(*main, GWL_EXSTYLE);
    style |= WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLongPtr(*main, GWL_STYLE, style);
    ShowWindow(*main, SW_MINIMIZE);
    SetWindowPos(*main, HWND_BOTTOM, 1, 1, 1, 1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    Sleep(300);
    ShowWindow(*main, SW_SHOWNOACTIVATE);
    SetWindowPos(*main, HWND_BOTTOM, 1, 1, 900, 600+34, SWP_NOZORDER | SWP_NOACTIVATE| SWP_FRAMECHANGED);
    Sleep(300);
    if (!SetWindowPos(*game, HWND_BOTTOM, 0, 34, 900, 600, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) {
        emit Logging("Если не сработала кнопка полной починки, сверните окно, используя кнопку \" - \" справа сверху у эмулятора"
                     "или воспользуйтесь комбинацией клавиш Win + D, а затем нажмите кнопку ещё раз.");
        return;
    }
    Sleep(300);
}
