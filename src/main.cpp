#include "ScriptHookV/main.h"
#include "ScriptHookV/natives.h"
#include "Superman.h"
#include <windows.h>

static SupermanController g_superman;

static bool KeyPressed(int key)
{
    static SHORT previous[256] = {};

    SHORT current = GetAsyncKeyState(key);

    bool pressed =
        (current & 0x8000) &&
        !(previous[key] & 0x8000);

    previous[key] = current;

    return pressed;
}

static void Notify(const char* text)
{
    OutputDebugStringA("[SUPERMAN] ");
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
}

static void UpdateControls()
{
    // F5 — Superman ON/OFF
    if (KeyPressed(VK_F5))
    {
        g_superman.enabled = !g_superman.enabled;

        if (g_superman.enabled)
        {
            Notify("SUPERMAN ON");
        }
        else
        {
            g_superman.Reset();
            Notify("SUPERMAN OFF");
        }
    }

    if (!g_superman.enabled)
        return;

    // F — Flight
    if (KeyPressed('F'))
    {
        g_superman.abilities.state.flight =
            !g_superman.abilities.state.flight;

        if (g_superman.abilities.state.flight)
            Notify("FLIGHT ON");
        else
            Notify("FLIGHT OFF");
    }

    // Left Shift — Boost
    g_superman.abilities.state.boost =
        (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;

    // G — Super Speed
    if (KeyPressed('G'))
    {
        g_superman.abilities.state.superSpeed =
            !g_superman.abilities.state.superSpeed;

        if (g_superman.abilities.state.superSpeed)
            Notify("SUPER SPEED ON");
        else
            Notify("SUPER SPEED OFF");
    }

    // H — Heat Vision
    if (KeyPressed('H'))
    {
        g_superman.abilities.state.heatVision =
            !g_superman.abilities.state.heatVision;

        if (g_superman.abilities.state.heatVision)
            Notify("HEAT VISION ON");
        else
            Notify("HEAT VISION OFF");
    }

    // J — Freeze Breath
    if (KeyPressed('J'))
    {
        g_superman.abilities.state.freezeBreath =
            !g_superman.abilities.state.freezeBreath;

        if (g_superman.abilities.state.freezeBreath)
            Notify("FREEZE BREATH ON");
        else
            Notify("FREEZE BREATH OFF");
    }

    // K — Super Breath
    if (KeyPressed('K'))
    {
        g_superman.abilities.state.superBreath =
            !g_superman.abilities.state.superBreath;

        if (g_superman.abilities.state.superBreath)
            Notify("SUPER BREATH ON");
        else
            Notify("SUPER BREATH OFF");
    }

    // B — Bullet Time
    if (KeyPressed('B'))
    {
        g_superman.abilities.state.bulletTime =
            !g_superman.abilities.state.bulletTime;

        if (g_superman.abilities.state.bulletTime)
            Notify("BULLET TIME ON");
        else
            Notify("BULLET TIME OFF");
    }

    // E — Grab
    if (KeyPressed('E'))
    {
        g_superman.abilities.state.grabbing =
            !g_superman.abilities.state.grabbing;

        if (g_superman.abilities.state.grabbing)
            Notify("GRAB ON");
        else
            Notify("GRAB OFF");
    }
}

void ScriptMain()
{
    Notify("SUPERMAN ASI STARTED");

    while (true)
    {
        UpdateControls();

        g_superman.Tick(0.016f);

        WAIT(0);
    }
}
