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

static void DrawTextSimple(const char* text, float x, float y, float scale)
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

    GRAPHICS::DRAW_RECT(
        0.18f,
        0.34f,
        0.32f,
        0.43f,
        0,
        0,
        0,
        210
    );

    GRAPHICS::DRAW_RECT(
        0.18f,
        0.135f,
        0.32f,
        0.055f,
        20,
        60,
        150,
        240
    );

    DrawTextSimple(
        "SUPERMAN",
        0.055f,
        0.112f,
        0.50f
    );

    for (int i = 0; i < MENU_COUNT; ++i)
    {
        float y = 0.165f + (i * 0.045f);

        if (i == g_selected)
        {
            GRAPHICS::DRAW_RECT(
                0.18f,
                y + 0.012f,
                0.29f,
                0.038f,
                40,
                100,
                200,
                220
            );
        }

        DrawTextSimple(
            MENU_ITEMS[i],
            0.055f,
            y,
            0.32f
        );
    }

    DrawTextSimple(
        "F3  CLOSE",
        0.055f,
        0.535f,
        0.25f
    );

    DrawTextSimple(
        "UP/DOWN  SELECT",
        0.055f,
        0.565f,
        0.25f
    );

    DrawTextSimple(
        "ENTER  ACTIVATE",
        0.055f,
        0.595f,
        0.25f
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
        case 0:
        {
            ToggleSuperman(ped);
            break;
        }

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

        case 2:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.boost =
                !g_superman.abilities.state.boost;

            break;
        }

        case 3:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.superSpeed =
                !g_superman.abilities.state.superSpeed;

            break;
        }

        case 4:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            break;
        }

        case 5:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.godMode =
                !g_superman.abilities.state.godMode;

            ENTITY::SET_ENTITY_INVINCIBLE(
                ped,
                g_superman.abilities.state.godMode
            );

            break;
        }

        case 6:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.heatVision =
                !g_superman.abilities.state.heatVision;

            break;
        }

        case 7:
        {
            if (!g_superman.enabled)
                EnableSuperman(ped);

            g_superman.abilities.state.freezeBreath =
                !g_superman.abilities.state.freezeBreath;

            break;
        }

        default:
            break;
    }
}

static void UpdateMenu(Ped ped)
{
    if (KeyPressed(VK_F3, g_lastF3))
    {
        g_menuOpen = !g_menuOpen;
    }

    if (!g_menuOpen)
        return;

    if (KeyPressed(VK_UP, g_lastUp))
    {
        --g_selected;

        if (g_selected < 0)
            g_selected = MENU_COUNT - 1;
    }

    if (KeyPressed(VK_DOWN, g_lastDown))
    {
        ++g_selected;

        if (g_selected >= MENU_COUNT)
            g_selected = 0;
    }

    if (KeyPressed(VK_RETURN, g_lastEnter))
    {
        ActivateMenuItem(ped);
    }
}

static void UpdateSuperJump(Ped ped)
{
    if (!g_superman.enabled)
        return;

    if (g_selected != 4)
        return;

    if (!g_menuOpen)
        return;

    if (GetAsyncKeyState(VK_RETURN) & 0x8000)
    {
        PLAYER::SET_SUPER_JUMP_THIS_FRAME();
    }
}

static void UpdateFlight(Ped ped)
{
    if (!g_superman.enabled)
        return;

    if (!g_superman.abilities.state.flight)
        return;

    Vector3 forward =
        ENTITY::GET_ENTITY_FORWARD_VECTOR(ped);

    float speed =
        g_superman.abilities.state.boost
        ? g_superman.boostSpeed
        : g_superman.flightSpeed;

    ENTITY::SET_ENTITY_VELOCITY(
        ped,
        forward.x * speed,
        forward.y * speed,
        forward.z * speed
    );
}

static void UpdateActiveAbilities(Ped ped)
{
    if (!g_superman.enabled)
        return;

    if (g_superman.abilities.state.godMode)
    {
        ENTITY::SET_ENTITY_INVINCIBLE(
            ped,
            true
        );
    }

    if (g_superman.abilities.state.superSpeed)
    {
        PED::SET_PED_MOVE_RATE_OVERRIDE(
            ped,
            2.0f
        );
    }
    else
    {
        PED::SET_PED_MOVE_RATE_OVERRIDE(
            ped,
            1.0f
        );
    }

    UpdateFlight(ped);

    g_superman.Tick(0.016f);
}

static void UpdateSuperman()
{
    Ped ped = PLAYER::PLAYER_PED_ID();

    if (!ped)
        return;

    UpdateMenu(ped);
    UpdateSuperJump(ped);
    UpdateActiveAbilities(ped);
    DrawMenu();
}

void ScriptMain()
{
    g_superman.Reset();

    while (true)
    {
        UpdateSuperman();
        WAIT(0);
    }
}
