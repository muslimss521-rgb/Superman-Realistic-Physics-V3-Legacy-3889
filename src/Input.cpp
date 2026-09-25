#include "Input.h"
#include <windows.h>

namespace
{
    constexpr int KEY_COUNT = 10;

    struct KeyState
    {
        int vk;
        bool down;
        bool previous;
    };

    KeyState g_keys[KEY_COUNT] =
    {
        { VK_F3,       false, false },
        { VK_F5,       false, false },
        { VK_SHIFT,    false, false },
        { 'W',         false, false },
        { 'S',         false, false },
        { 'A',         false, false },
        { 'D',         false, false },
        { VK_SPACE,    false, false },
        { VK_CONTROL,  false, false },
        { VK_F8,       false, false }
    };
}

namespace Input
{
    void Update()
    {
        for (auto& key : g_keys)
        {
            key.previous = key.down;
            key.down = (GetAsyncKeyState(key.vk) & 0x8000) != 0;
        }
    }

    bool Down(Key key)
    {
        return g_keys[static_cast<int>(key)].down;
    }

    bool Pressed(Key key)
    {
        const auto& k = g_keys[static_cast<int>(key)];
        return k.down && !k.previous;
    }
}
