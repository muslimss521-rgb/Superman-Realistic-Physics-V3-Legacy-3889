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

            Player player = PLAYER::PLAYER_ID();
            Ped ped = PLAYER::PLAYER_PED_ID();

            if (g_superman.enabled)
            {
                g_superman.abilities.state.flight = true;
                g_superman.abilities.state.superSpeed = true;
                g_superman.abilities.state.godMode = true;
                g_superman.abilities.state.heatVision = true;
                g_superman.abilities.state.freezeBreath = true;
                g_superman.abilities.state.superBreath = true;

                ENTITY::SET_ENTITY_INVINCIBLE(
                    ped,
                    true
                );

                PLAYER::SET_PLAYER_INVINCIBLE(
                    player,
                    true
                );

                HUD::SET_NOTIFICATION_TEXT_ENTRY("STRING");
                HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(
                    "SUPERMAN ON"
                );
                HUD::DRAW_NOTIFICATION(
                    false,
                    false
                );
            }
            else
            {
                g_superman.Reset();

                ENTITY::SET_ENTITY_INVINCIBLE(
                    ped,
                    false
                );

                PLAYER::SET_PLAYER_INVINCIBLE(
                    player,
                    false
                );

                HUD::SET_NOTIFICATION_TEXT_ENTRY("STRING");
                HUD
