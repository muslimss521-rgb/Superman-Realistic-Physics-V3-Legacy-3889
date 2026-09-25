#pragma once

namespace Input
{
    enum class Key
    {
        Toggle = 0,
        Flight,
        Boost,
        Forward,
        Back,
        Left,
        Right,
        Up,
        Down,
        Emergency
    };

    void Update();
    bool Down(Key key);
    bool Pressed(Key key);
}
