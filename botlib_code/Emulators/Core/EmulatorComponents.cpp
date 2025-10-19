#include "Emulators/Core/EmulatorComponents.h"

#include <regex>
#include <comdef.h>
#include <wbemidl.h>

#include <QString>
#include <QThread>

#pragma comment(lib, "wbemuuid.lib")

std::wstring GetCommandLineForHWND(HWND hwnd) {

    DWORD processID;
    GetWindowThreadProcessId(hwnd, &processID);

    HRESULT hres;

    // Инициализация COM
    hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hres)) return L"";

    // Настройка безопасности
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
        );

    if (FAILED(hres)) {
        CoUninitialize();
        return L"";
    }

    // Получение указателя на WMI
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
        );

    if (FAILED(hres)) {
        CoUninitialize();
        return L"";
    }

    IWbemServices* pSvc = NULL;

    // Подключение к WMI
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
        );

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return L"";
    }

    // Установка уровня безопасности для WMI
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
        );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return L"";
    }

    // Формируем запрос для WMI
    IEnumWbemClassObject* pEnumerator = NULL;
    std::wstring query = L"SELECT CommandLine FROM Win32_Process WHERE ProcessId = " + std::to_wstring(processID);

    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
        );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return L"";
    }

    // Читаем результат запроса
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    std::wstring commandLine;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn) break;

        VARIANT vtProp;
        hr = pclsObj->Get(L"CommandLine", 0, &vtProp, 0, 0);

        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) commandLine = vtProp.bstrVal; // Сохраняем результат

        VariantClear(&vtProp);
        pclsObj->Release();
    }
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return commandLine;
}

std::wstring sanitizePath(const std::wstring &input, typeEmu type, int &instance) {
    std::wstring result;
    result.reserve(input.size());

    std::wstring temp;
    for (wchar_t ch : input) {
        if (ch == L'"') continue;
        else temp += ch;
    }

    switch (type) {
        case typeEmu::ld_player: {
        // Заменяем dnplayer на ldconsole
        size_t pos = temp.find(L"dnplayer");
        if (pos != std::wstring::npos) temp.replace(pos, wcslen(L"dnplayer"), L"ldconsole");

        // Извлекаем число между = и |
        std::wregex pattern(L"index=(\\d+)\\|");
        std::wsmatch match;
        if (regex_search(temp, match, pattern)) {
            instance = stoi(match[1].str()); // Преобразуем найденное число в int
            temp = regex_replace(temp, pattern, L""); // Удаляем "index=1|" из строки
        } else {
            instance = 0; // Указываем ошибку, если число не найдено
            return L"";
        }
        result = temp;
        return result;
        }
        case typeEmu::nox_app: {
            return L"";
        }
        case typeEmu::memu_app: {
            return L"";
        }
        case typeEmu::mg_launcher: {
            return L"";
        }
        default: return L"";
    }

    return L"";
}

bool FindEmulator(const QString &windowName, HWND *main, HWND *game) {
    constexpr int INITIAL_DELAY_MS = 500;
    constexpr int GAME_DELAY_MS = 300;
    int addCount = 34;
    QThread::msleep(INITIAL_DELAY_MS);
    int x = 0;
    while (!(*main = FindWindowA(NULL, windowName.toUtf8().constData()))) {
        QThread::msleep(INITIAL_DELAY_MS);
        x++;
        if (x >= 100) return false;
    }
    x = 0;
    //потом добавить буль флаг и сделать проверку не только на RenderWindow внутри одного while
    //и вместе с разными RenderWindow менять addCount
    while (!(*game = FindWindowExA(*main, NULL,"RenderWindow",NULL))) {
        QThread::msleep(GAME_DELAY_MS);
        x++;
        if (x >= 300) return false;
    }
    // Установка параметров окна
    LONG_PTR style = GetWindowLongPtr(*main, GWL_EXSTYLE);
    style |= WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLongPtr(*main, GWL_STYLE, style);
    ShowWindow(*main, SW_MINIMIZE);
    if (!SetWindowPos(*main, HWND_BOTTOM, 1, 1, 1, 1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) return false;
    QThread::msleep(INITIAL_DELAY_MS);
    ShowWindow(*main, SW_SHOWNOACTIVATE);
    if (!SetWindowPos(*main, HWND_BOTTOM, 1, 1, 900, 600 + addCount, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) return false;
    QThread::msleep(INITIAL_DELAY_MS);
    if (!SetWindowPos(*game, HWND_BOTTOM, 0, addCount, 900, 600, SWP_NOZORDER | SWP_NOACTIVATE |SWP_FRAMECHANGED)) return false;
    QThread::msleep(INITIAL_DELAY_MS);
    return true;
}
