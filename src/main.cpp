#include <windows.h>
#include "../include/ScriptHookV/main.h"
#include "../include/ScriptHookV/natives.h"
#include "Superman.h"

static SupermanController g_superman;

DWORD WINAPI MainThread(LPVOID)
{
    bool lastF3 = false;

    while (true)
    {
        Sleep(16);

        bool f3 = (GetAsyncKeyState(VK_F3) & 0x8000) != 0;

        // Срабатывает один раз при нажатии F3.
        if (f3 && !lastF3)
        {
            g_superman.enabled = !g_superman.enabled;

            if (!g_superman.enabled)
            {
                g_superman.Reset();
            }
        }

        lastF3 = f3;

        // Пока Superman выключен, ничего не изменяем.
        if (!g_superman.enabled)
            continue;

        g_superman.Tick(0.016f);
    }

    return 0;
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD reason,
    LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        CreateThread(
            nullptr,
            0,
            MainThread,
            nullptr,
            0,
            nullptr
        );
    }

    return TRUE;
}
