#include "TargetLock.h"
#include "main.h"
#include "natives.h"

namespace TargetLock
{
    static bool g_active = false;
    static Ped g_target = 0;

    void Toggle()
    {
        g_active = !g_active;

        if (!g_active)
            g_target = 0;
    }

    bool Active()
    {
        return g_active;
    }

    void Update()
    {
        if (!g_active)
            return;

        Ped p = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(p))
        {
            g_target = 0;
            return;
        }

        // Target acquisition is intentionally conservative for this SDK.
        // Once a target exists, clear it if the entity disappears.
        if (g_target != 0 &&
            !ENTITY::DOES_ENTITY_EXIST(g_target))
        {
            g_target = 0;
        }
    }
}
