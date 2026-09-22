#include <windows.h>
#include "../include/ScriptHookV/main.h"
#include "../include/ScriptHookV/natives.h"
#include "Superman.h"

static SupermanController g_superman;

void ShowStatus()
{
    if (g_superman.enabled)
    {
        HUD::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");
        HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("SUPERMAN: ON");
        HUD::END_TEXT_COMMAND_THEFEED_POST_TICKER(false, false);
    }
    else
    {
        HUD::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");
        HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("SUPERMAN: OFF");
        HUD::END_TEXT_COMMAND_THEFEED_POST_TICKER(false, false);
    }
}

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

            ShowStatus();
        }

        lastF3 = f3;

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
