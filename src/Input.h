#pragma once

namespace Input
{
    bool Pressed(int key);
    bool JustPressed(int key);

    // Movement
    bool Forward();
    bool Back();
    bool Left();
    bool Right();
    bool Up();
    bool Down();
    bool Boost();

    // Superman abilities - letter keys
    bool ToggleSuperman();    // G
    bool ToggleFlight();      // F
    bool SuperPunch();        // R
    bool ToggleTargetLock();  // T
    bool EmergencyOff();      // X
    bool ToggleHeatVision();  // H
    bool ToggleSuperSpeed();  // C
}
