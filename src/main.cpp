#include <windows.h>

#include "ScriptHookV/main.h"
#include "ScriptHookV/natives.h"

#include "Superman.h"

static SupermanController g_superman;

static bool g_menuOpen = false;
static int g_selected = 0;

static bool g_lastF3 = false;
static bool g_lastEnter = false;
static bool g_lastUp = false;
static bool g_lastDown = false;

static const int MENU_COUNT = 8;

static const char* MENU_ITEMS[MENU_COUNT] =
{
    "Superman",
    "Flight",
    "Boost",
    "Super Speed",
    "Super Jump",
    "Invincibility",
    "Heat Vision",
    "Freeze Breath"
};

static bool KeyPressed(int key, bool& lastState)
{
    bool current = (GetAsyncKeyState(key) & 0x8000) != 0;

    bool pressed = current && !lastState;

    lastState = current;

    return pressed;
}

static void DrawText(const char* text, float x, float y, float scale)
{
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, scale);
    UI::SET_TEXT_COLOUR(255, 255, 255, 255);
    UI::SET_TEXT_PROPORTIONAL(true);
    UI::SET_TEXT_OUTLINE();

    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING((char*)text);
    UI::_DRAW_TEXT(x, y);
}

static void DrawMenu()
{
    if (!g_menuOpen)
        return;

    /*
        Main background
    */
    GRAPHICS::DRAW_RECT(
        0.18f,
        0.32f,
        0.30f,
        0.40f,
        0,
        0,
        0,
        190
    );

    /*
        Header
    */
    GRAPHICS::DRAW_RECT(
        0.18f,
        0.135f,
        0.30f,
        0.055f,
        20,
        60,
        150,
        240
    );

    DrawText(
        "SUPERMAN",
        0.055f,
        0.115f,
        0.55f
    );

    /*
        Menu entries
    */
    for (int i = 0; i < MENU_COUNT; i++)
    {
        float y = 0.165f + (i * 0.045f);

        if (i == g_selected)
        {
            GRAPHICS::DRAW_RECT(
                0.18f,
                y + 0.012f,
                0.27f,
                0.038f,
                40,
                100,
                200,
                220
            );
        }

        DrawText(
            MENU_ITEMS[i],
            0.055f,
            y,
            0.34f
        );
    }

    DrawText(
        "F3  Close",
        0.055f,
        0.535f,
        0.27f
    );

    DrawText(
        "UP/DOWN  Select",
        0.055f,
        0.565f,
        0.27f
    );

    DrawText(
        "ENTER  Activate",
        0.055f,
        0.595f,
        0.27f
    );
}

static void EnableSuperman(Ped ped)
{
    g_superman.enabled = true;

    ENTITY::SET_ENTITY_INVINCIBLE(
        ped,
        true
    );

    g_superman.abilities.state.superSpeed = true;

    PED::SET_PED_MOVE_RATE_OVERRIDE(
        ped,
        2.0f
    );
}

static void DisableSuperman(Ped ped)
{
    g_superman.enabled = false;

    g_superman.abilities.state.flight = false;
    g_superman.abilities.state.boost = false;
    g_superman.abilities.state.superSpeed = false;
    g_superman.abilities.state.godMode = false;

    ENTITY::SET_ENTITY_INVINCIBLE(
        ped,
        false
    );

    PED::SET_PED_MOVE_RATE_OVERRIDE(
        ped,
        1.0f
    );

    ENTITY::SET_ENTITY_HAS_GRAVITY(
        ped,
        true
    );

    g_superman.velocity = Vec3();
}

static void ToggleSuperman(Ped ped)
{
    if (g_superman.enabled)
        DisableSuperman(ped);
    else
        EnableSuperman(ped);
}

static void ActivateMenuItem(Ped ped)
{
    switch (g_selected)
    {
        /*
            Superman
        */
        case 0:
        {
            ToggleSuperman(ped);
            break;
        }

        /*
            Flight
        */
        case 1:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.flight =
                !g_superman.abilities.state.flight;

            if (g_superman.abilities.state.flight)
            {
                ENTITY::SET_ENTITY_HAS_GRAVITY(
                    ped,
                    false
                );
            }
            else
            {
                ENTITY::SET_ENTITY_HAS_GRAVITY(
                    ped,
                    true
                );

                ENTITY::SET_ENTITY_VELOCITY(
                    ped,
                    0.0f,
                    0.0f,
                    0.0f
                );
            }

            break;
        }

        /*
            Boost
        */
        case 2:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.boost =
                !g_superman.abilities.state.boost;

            break;
        }

        /*
            Super Speed
        */
        case 3:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.superSpeed =
                !g_superman.abilities.state.superSpeed;

            if (g_superman.
