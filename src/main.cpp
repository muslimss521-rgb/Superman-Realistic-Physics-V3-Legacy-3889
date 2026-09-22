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

void ShowNotification(const char* text)
{
    OutputDebugStringA("[SUPERMAN] ");
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
}

void UpdateControls()
{
    // F3 — Superman ON/OFF
    if (KeyPressed(VK_F3))
    {
        g_superman.enabled = !g_superman.enabled;

        if (!g_superman.enabled)
        {
            g_superman.Reset();
            ShowNotification("Superman OFF");
        }
        else
        {
            ShowNotification("Superman ON");
        }
    }

    // Если Superman выключен — остальные клавиши ничего не делают.
    if (!g_superman.enabled)
        return;

    // F — Flight
    if (KeyPressed('F'))
    {
        g_superman.abilities.state.flight =
            !g_superman.abilities.state.flight;

        ShowNotification(
            g_superman.abilities.state.flight
            ? "Flight ON"
            : "Flight OFF"
        );
    }

    // Left Shift — Boost
    if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
    {
        g_superman.abilities.state.boost = true;
    }
    else
    {
        g_superman.abilities.state.boost = false;
    }

    // G — Super Speed
    if (KeyPressed('G'))
    {
        g_superman.abilities.state.superSpeed =
            !g_superman.abilities.state.superSpeed;

        ShowNotification(
            g_superman.abilities.state.superSpeed
            ? "Super Speed ON"
            : "Super Speed OFF"
        );
    }

    // H — Heat Vision
    if (KeyPressed('H'))
    {
        g_superman.abilities.state.heatVision =
            !g_superman.abilities.state.heatVision;

        ShowNotification(
            g_superman.abilities.state.heatVision
            ? "Heat Vision ON"
            : "Heat Vision OFF"
        );
    }

    // J — Freeze Breath
    if (KeyPressed('J'))
    {
        g_superman.abilities.state.freezeBreath =
            !g_superman.abilities.state.freezeBreath;

        ShowNotification(
            g_superman.abilities.state.freezeBreath
            ? "Freeze Breath ON"
            : "Freeze Breath OFF"
        );
    }

    // K — Super Breath
    if (KeyPressed('K'))
    {
        g_superman.abilities.state.superBreath =
            !g_superman.abilities.state.superBreath;

        ShowNotification(
            g_superman.abilities.state.superBreath
            ? "Super Breath ON"
            : "Super Breath OFF"
        );
    }

    // B — Bullet Time
    if (KeyPressed('B'))
    {
        g_superman.abilities.state.bulletTime =
            !g_superman.abilities.state.bulletTime;

        ShowNotification(
            g_superman.abilities.state.bulletTime
            ? "Bullet Time ON"
            : "Bullet Time OFF"
        );
    }

    // E — Grab
    if (KeyPressed('E'))
    {
        g_superman.abilities.state.grabbing =
            !g_superman.abilities.state.grabbing;

        ShowNotification(
            g_superman.abilities.state.grabbing
            ? "Grab ON"
            : "Grab OFF"
        );
    }
}

void SupermanMain()
{
    static bool initialized = false;
    static ULONGLONG lastTime = 0;

    if (!initialized)
    {
        initialized = true;
        lastTime = GetTickCount64();

        ShowNotification("Superman mod loaded!");
    }

    ULONGLONG currentTime = GetTickCount64();

    float dt =
        static_cast<float>(currentTime - lastTime) / 1000.0f;

    lastTime = currentTime;

    // Защита от слишком большого скачка времени.
    if (dt > 0.1f)
        dt = 0.1f;

    UpdateControls();

    g_superman.Tick(dt);
}
