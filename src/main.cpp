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

        Notify(
            g_superman.abilities.state.flight
                ? "FLIGHT ON"
                : "FLIGHT OFF"
