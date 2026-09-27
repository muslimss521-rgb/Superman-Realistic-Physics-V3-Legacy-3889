#include "Input.h"
#include <windows.h>

namespace Input
{
    static bool KeyDown(int key)
    {
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    }

    bool Pressed(int key)
    {
        return KeyDown(key);
    }

    bool JustPressed(int key)
    {
        static bool previous[256] = {};

        if (key < 0 || key > 255)
            return false;

        const bool now = KeyDown(key);
        const bool result = now && !previous[key];
        previous[key] = now;
        return result;
    }

    bool Forward() { return KeyDown('W'); }
    bool Back()    { return KeyDown('S'); }
    bool Left()    { return KeyDown('A'); }
    bool Right()   { return KeyDown('D'); }
    bool Up()      { return KeyDown(VK_SPACE); }
    bool Down()    { return KeyDown(VK_LCONTROL) || KeyDown(VK_RCONTROL); }
    bool Boost()   { return KeyDown(VK_LSHIFT) || KeyDown(VK_RSHIFT); }

    // F3 = master Superman ON/OFF.
    bool ToggleSuperman()    { return JustPressed(VK_F3); }

    // F = flight ON/OFF.
    bool ToggleFlight()      { return JustPressed('F'); }

    // R = super punch.
    bool SuperPunch()        { return JustPressed('R'); }

    // T = target lock.
    bool ToggleTargetLock()  { return JustPressed('T'); }

    // X = emergency OFF.
    bool EmergencyOff()      { return JustPressed('X'); }

    // H = heat vision.
    bool ToggleHeatVision()  { return JustPressed('H'); }

    // C = super speed.
    bool ToggleSuperSpeed()  { return JustPressed('C'); }
}
