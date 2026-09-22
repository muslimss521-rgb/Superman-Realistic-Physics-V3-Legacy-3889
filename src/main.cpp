#include "Superman.h"

#include "main.h"
#include "natives.h"

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

        Notify(
            g_superman.abilities.state.flight
                ? "FLIGHT ON"
                : "FLIGHT OFF"
        );
    }

    // Left Shift — Boost
    g_superman.abilities.state.boost =
        (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;

    // G — Super Speed
    if (KeyPressed('G'))
    {
        g_superman.abilities.state.superSpeed =
            !g_superman.abilities.state.superSpeed;

        Notify(
            g_superman.abilities.state.superSpeed
                ? "SUPER SPEED ON"
                : "SUPER SPEED OFF"
        );
    }

    // H — Heat Vision
    if (KeyPressed('H'))
    {
        g_superman.abilities.state.heatVision =
            !g_superman.abilities.state.heatVision;

        Notify(
            g_superman.abilities.state.heatVision
                ? "HEAT VISION ON"
                : "HEAT VISION OFF"
        );
    }

    // J — Freeze Breath
    if (KeyPressed('J'))
    {
        g_superman.abilities.state.freezeBreath =
            !g_superman.abilities.state.freezeBreath;

        Notify(
            g_superman.abilities.state.freezeBreath
                ? "FREEZE BREATH ON"
                : "FREEZE BREATH OFF"
        );
    }

    // K — Super Breath
    if (KeyPressed('K'))
    {
        g_superman.abilities.state.superBreath =
            !g_superman.abilities.state.superBreath;

        Notify(
            g_superman.abilities.state.superBreath
                ? "SUPER BREATH ON"
                : "SUPER BREATH OFF"
        );
    }

    // B — Bullet Time
    if (KeyPressed('B'))
    {
        g_superman.abilities.state.bulletTime =
            !g_superman.abilities.state.bulletTime;

        Notify(
            g_superman.abilities.state.bulletTime
                ? "BULLET TIME ON"
                : "BULLET TIME OFF"
        );
    }

    // E — Grab
    if (KeyPressed('E'))
    {
        g_superman.abilities.state.grabbing =
            !g_superman.abilities.state.grabbing;

        Notify(
            g_superman.abilities.state.grabbing
                ? "GRAB ON"
                : "GRAB OFF"
        );
    }
}

static void Tick()
{
    static ULONGLONG lastTime = GetTickCount64();

    ULONGLONG currentTime = GetTickCount64();

    float dt =
        static_cast<float>(currentTime - lastTime) /
        1000.0f;

    lastTime = currentTime;

    if (dt > 0.1f)
        dt = 0.1f;

    UpdateControls();

    g_superman.Tick(dt);
}

void ScriptMain()
{
    Notify("SUPERMAN ASI STARTED");

    while (true)
    {
        Tick();

        WAIT(0);
    }
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    return TRUE;
}
