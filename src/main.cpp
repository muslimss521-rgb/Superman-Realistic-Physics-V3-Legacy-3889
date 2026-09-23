#include <windows.h>

#include "../include/ScriptHookV/main.h"
#include "../include/ScriptHookV/natives.h"

#include "Superman.h"

static SupermanController g_superman;

void SupermanMain()
{
    bool lastF5 = false;

    while (true)
    {
        bool f5 = (GetAsyncKeyState(VK_F5) & 0x8000) != 0;

        if (f5 && !lastF5)
        {
            g_superman.enabled = !g_superman.enabled;

            if (g_superman.enabled)
            {
                g_superman.abilities.state.flight = true;
                g_superman.abilities.state.superSpeed = true;
                g_superman.abilities.state.godMode = true;
                g_superman.abilities.state.heatVision = true;
                g_superman.abilities.state.freezeBreath = true;
                g_superman.abilities.state.superBreath = true;
                g_superman.abilities.state.boost = false;
            }
            else
            {
                g_superman.Reset();
            }
        }

        lastF5 = f5;

        if (g_superman.enabled)
        {
            g_superman.Tick(0.016f);
        }

        scriptWait(0);
    }
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD reason,
    LPVOID)
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
