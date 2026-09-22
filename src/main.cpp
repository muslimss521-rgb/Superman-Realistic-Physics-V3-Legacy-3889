#include "Superman.h"
#include <windows.h>

void ShowNotification(const char* text)
{
    // Пока без HUD API — безопасно для текущего Native-слоя.
    // Выводим сообщение в debugger/output.
    OutputDebugStringA("[SUPERMAN] ");
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
}

void SupermanMain()
{
    static bool initialized = false;

    if (!initialized)
    {
        initialized = true;
        ShowNotification("Superman mod loaded!");
    }

    // Основная логика находится в Superman.cpp
}
