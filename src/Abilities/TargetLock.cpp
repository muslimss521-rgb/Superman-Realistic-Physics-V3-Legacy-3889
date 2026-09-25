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

    void Disable()
    {
        g_active = false;
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

        Ped player = PLAYER::PLAYER_PED_ID();
        if (!ENTITY::DOES_ENTITY_EXIST(player))
        {
            g_target = 0;
            return;
        }

        if (g_target == 0 || !ENTITY::DOES_ENTITY_EXIST(g_target))
        {
            Vector3 p = ENTITY::GET_ENTITY_COORDS(player, TRUE);
            Ped target = 0;

            if (PED::GET_CLOSEST_PED(
                    p.x, p.y, p.z,
                    40.0f,
                    TRUE, TRUE,
                    &target,
                    FALSE, FALSE,
                    4) &&
                target != 0 &&
                target != player &&
                ENTITY::DOES_ENTITY_EXIST(target))
            {
                g_target = target;
            }
        }

        if (g_target != 0 && ENTITY::DOES_ENTITY_EXIST(g_target))
        {
            Vector3 a = ENTITY::GET_ENTITY_COORDS(player, TRUE);
            Vector3 b = ENTITY::GET_ENTITY_COORDS(g_target, TRUE);

            GRAPHICS::DRAW_LINE(
                a.x, a.y, a.z + 1.0f,
                b.x, b.y, b.z + 1.0f,
                255, 255, 0, 220);

            ENTITY::SET_ENTITY_IS_TARGET_PRIORITY(
                g_target,
                TRUE,
                1.0f);
        }
    }
}
