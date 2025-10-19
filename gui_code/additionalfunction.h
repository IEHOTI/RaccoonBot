#ifndef ADDITIONALFUNCTION_H
#define ADDITIONALFUNCTION_H

#include "qcontainerfwd.h"
#include <windows.h>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void getEmulatorsList(QList<HWND>& hwndList, QList<QString>& nameList);
#endif // ADDITIONALFUNCTION_H
