#include <windows.h>
#include "../include/ScriptHookV/main.h"
#include "../include/ScriptHookV/natives.h"
#include "Superman.h"

static SupermanController g_superman;

void SupermanMain()
{
    bool lastF3 = false;

    while (true)
    {
        bool f3 = (GetAsyncKeyState(VK_F3) & 0x8000) != 0;

        if (f3 && !lastF3)
        {
            g_superman.enabled = !g_superman.enabled;

            if (!g_superman.enabled)
            {
                g_superman.Reset();
            }
        }

        lastF3 = f3;

        if (g_superman.enabled)
        {
            g_superman.Tick(0.016f);
        }

        // Передаём управление обратно GTA.
        scriptWait(0);
    }
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD reason,
    LPVOID
)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        scriptRegister(hModule, SupermanMain);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        scriptUnregister(hModule);
    }

    return TRUE;
}
