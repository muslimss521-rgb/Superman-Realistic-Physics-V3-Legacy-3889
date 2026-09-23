#include <windows.h>

#include "ScriptHookV/main.h"
#include "ScriptHookV/natives.h"
#include "Superman.h"

static SupermanController g_superman;

static bool g_menuOpen = false;
static int g_selected = 0;

static bool lastF3 = false;
static bool lastUp = false;
static bool lastDown = false;
static bool lastEnter = false;

static const int MENU_COUNT = 8;

static const char* MENU[MENU_COUNT] =
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

static bool Pressed(int key, bool& last)
{
    bool now = (GetAsyncKeyState(key) & 0x8000) != 0;
    bool result = now && !last;
    last = now;
    return result;
}

static void Text(
    const char* text,
    float x,
    float y,
    float scale)
{
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, scale);
    UI::SET_TEXT_COLOUR(
        255,
        255,
        255,
        255
    );
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
        0.33f,
        0.32f,
        0.40f,
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
        25,
        70,
        160,
        240
    );

    Text(
        "SUPERMAN",
        0.055f,
        0.112f,
        0.50f
    );

    for (int i = 0; i < MENU_COUNT; ++i)
    {
        float y = 0.165f + i * 0.043f;

        if (i == g_selected)
        {
            GRAPHICS::DRAW_RECT(
                0.18f,
                y + 0.011f,
                0.29f,
                0.036f,
                50,
                110,
                210,
                220
            );
        }

        Text(
            MENU[i],
            0.055f,
            y,
            0.31f
        );
    }

    Text(
        "F3  CLOSE",
        0.055f,
        0.535f,
        0.24f
    );

    Text(
        "UP/DOWN  SELECT",
        0.055f,
        0.562f,
        0.24f
    );

    Text(
        "ENTER  ACTIVATE",
        0.055f,
        0.589f,
        0.24f
    );
}

static void SupermanOn(Ped ped)
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

static void SupermanOff(Ped ped)
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

    ENTITY::SET_ENTITY_HAS_GRAVITY(
        ped,
        true
    );

    PED::SET_PED_MOVE_RATE_OVERRIDE(
        ped,
        1.0f
    );

    ENTITY::SET_ENTITY_VELOCITY(
        ped,
        0.0f,
        0.0f,
        0.0f
    );

    g_superman.velocity = Vec3();
}

static void Activate(Ped ped)
{
    switch (g_selected)
    {
        case 0:
        {
            if (g_superman.enabled)
                SupermanOff(ped);
            else
                SupermanOn(ped);

            break;
        }

        case 1:
        {
            if (!g_superman.enabled)
                SupermanOn(ped);

            g_superman.abilities.state.flight =
                !g_superman.abilities.state.flight;

            ENTITY::SET_ENTITY_HAS_GRAVITY(
                ped,
                !g_superman.abilities.state.flight
            );

            break;
        }

        case 2:
        {
            if (!g_superman.enabled)
                SupermanOn(ped);

            g_superman.abilities.state.boost =
                !g_superman.abilities.state.boost;

            break;
        }

        case 3:
        {
            if (!g_superman.enabled)
                SupermanOn(ped);

            g_superman.abilities.state.superSpeed =
                !g_superman.abilities.state.superSpeed;

            break;
        }

        case 4:
        {
            if (!g_superman.enabled)
                SupermanOn(ped);

            break;
        }

        case 5:
        {
            if (!g_superman.enabled)
                SupermanOn(ped);

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
                SupermanOn(ped);

            g_superman.abilities.state.heatVision =
                !g_superman.abilities.state.heatVision;

            break;
        }

        case 7:
        {
            if (!g_superman.enabled)
                SupermanOn(ped);

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
    if (Pressed(VK_F3, lastF3))
        g_menuOpen = !g_menuOpen;

    if (!g_menuOpen)
        return;

    if (Pressed(VK_UP, lastUp))
    {
        --g_selected;

        if (g_selected < 0)
            g_selected = MENU_COUNT - 1;
    }

    if (Pressed(VK_DOWN, lastDown))
    {
        ++g_selected;

        if (g_selected >= MENU_COUNT)
            g_selected = 0;
    }

    if (Pressed(VK_RETURN, lastEnter))
        Activate(ped);
}

static void UpdateSuperJump(Ped ped)
{
    if (!g_superman.enabled)
        return;

    if (!g_menuOpen)
        return;

    if (g_selected != 4)
        return;

    if (GetAsyncKeyState(VK_RETURN) & 0x8000)
    {
        GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(
            PLAYER::PLAYER_ID()
        );
    }
}

static void UpdateFlight(Ped ped)
{
    if (!g_superman.enabled)
        return;

    if (!g_superman.abilities.state.flight)
        return;

    Vector3 forward =
        ENTITY::GET_ENTITY_FORWARD_VECTOR(
            ped
        );

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

static void UpdateAbilities(Ped ped)
{
    if (!g_superman.enabled)
        return;

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

    if (g_superman.abilities.state.godMode)
    {
        ENTITY::SET_ENTITY_INVINCIBLE(
            ped,
            true
        );
    }

    UpdateFlight(ped);

    g_superman.Tick(0.016f);
}

static void MainUpdate()
{
    Ped ped = PLAYER::PLAYER_PED_ID();

    if (!ped)
        return;

    UpdateMenu(ped);
    UpdateSuperJump(ped);
    UpdateAbilities(ped);
    DrawMenu();
}

void ScriptMain()
{
    g_superman.Reset();

    while (true)
    {
        MainUpdate();
        WAIT(0);
    }
}
