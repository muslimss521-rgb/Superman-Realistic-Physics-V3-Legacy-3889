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
        bool f3 =
            (GetAsyncKeyState(VK_F3) & 0x8000) != 0;

        if (f3 && !lastF3)
        {
            g_superman.enabled =
                !g_superman.enabled;

            g_superman.abilities.state.superSpeed =
                g_superman.enabled;
        }

        lastF3 = f3;

        if (g_superman.enabled)
        {
            Ped ped = PLAYER::PLAYER_PED_ID();

            // Увеличиваем скорость передвижения игрока
            PED::SET_PED_MOVE_RATE_OVERRIDE(
                ped,
                2.0f
            );

            g_superman.Tick(0.016f);
        }
        else
        {
            Ped ped = PLAYER::PLAYER_PED_ID();

            PED::SET_PED_MOVE_RATE_OVERRIDE(
                ped,
                1.0f
            );
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
