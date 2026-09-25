#include "Input.h"
#include "main.h"

namespace Input {
    bool JustPressed(int c) { return PAD::IS_CONTROL_JUST_PRESSED(0, c); }
    bool Pressed(int c) { return PAD::IS_CONTROL_PRESSED(0, c); }
    bool Forward() { return Pressed(32); } // W
    bool Back() { return Pressed(33); }    // S
    bool Left() { return Pressed(34); }    // A
    bool Right() { return Pressed(35); }   // D
    bool Up() { return Pressed(22); }      // Space
    bool Down() { return Pressed(36); }    // Ctrl
    bool Boost() { return Pressed(21); }   // Shift
}
