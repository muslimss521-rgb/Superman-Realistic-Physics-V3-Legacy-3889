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
        const bool now = KeyDown(key);
        const bool result = now && !previous[key];
        previous[key] = now;
        return result;
    }

    // Movement
    bool Forward() { return KeyDown('W'); }
    bool Back()    { return KeyDown('S'); }
    bool Left()    { return KeyDown('A'); }
    bool Right()   { return KeyDown('D'); }
    bool Up()      { return KeyDown(VK_SPACE); }
    bool Down()    { return KeyDown(VK_LCONTROL) || KeyDown(VK_RCONTROL); }
    bool Boost()   { return KeyDown(VK_LSHIFT) || KeyDown(VK_RSHIFT); }

    // Abilities
    bool ToggleSuperman()   { return JustPressed('G'); }
    bool ToggleFlight()     { return JustPressed('F'); }
    bool SuperPunch()       { return JustPressed('R'); }
    bool ToggleTargetLock() { return JustPressed('T'); }
    bool EmergencyOff()     { return JustPressed('X'); }
    bool ToggleHeatVision() { return JustPressed('H'); }
    bool ToggleSuperSpeed() { return JustPressed('C'); }
}
