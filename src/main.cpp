#include <windows.h>
#include "ScriptHookV/main.h"
#include "Superman.h"

static SupermanController g_superman;

DWORD WINAPI MainThread(LPVOID)
{
    while (true)
    {
        Sleep(16);

        // Обновляем физическое состояние Superman.
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
