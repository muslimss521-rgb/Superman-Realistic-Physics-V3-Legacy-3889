#include <windows.h>

#include "../include/ScriptHookV/main.h"
#include "../include/ScriptHookV/natives.h"

#include "Superman.h"

static SupermanController g_superman;

void ShowStatus()
{
    HUD::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");

    if (g_superman.enabled)
    {
        HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(
            "SUPERMAN: ON"
        );
    }
    else
    {
        HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(
            "SUPERMAN: OFF"
        );
    }

    HUD::END_TEXT_COMMAND_THEFEED_POST_TICKER(
        false,
        false
    );
}

void SupermanMain()
{
    bool lastF5 = false;

    while (true)
    {
        bool f5 =
            (GetAsyncKeyState(VK_F5) & 0x8000) != 0;

        if (f5 && !lastF5)
        {
            g_superman.enabled =
                !g_superman.enabled;

            if (g_superman.enabled)
            {
                g_superman.abilities.state.flight = true;
                g_superman.abilities.state.superSpeed = true;
                g_superman.abilities.state.godMode = true;
                g_superman.abilities.state.heatVision = true;
                g_superman.abilities.state.freezeBreath = true;
                g
