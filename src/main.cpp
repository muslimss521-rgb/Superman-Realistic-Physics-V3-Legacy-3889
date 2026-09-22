#include "ScriptHookV/main.h"
#include <windows.h>

static bool previousF5 = false;

static void Notify(const char* text)
{
    OutputDebugStringA("[SUPERMAN] ");
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
}

static void SupermanScript()
{
    Notify("SUPERMAN SCRIPT STARTED");

    while (true)
    {
        bool f5 =
            (GetAsyncKeyState(VK_F5) & 0x8000) != 0;

        if (f5 && !previousF5)
        {
            Notify("SUPERMAN F5 PRESSED");
        }

        previousF5 = f5;

        WAIT(0);
    }
}

BOOL WINAPI DllMain(
    HMODULE hModule,
    DWORD reason,
    LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        scriptRegister(
            hModule,
            SupermanScript
        );
    }

    return TRUE;
}
