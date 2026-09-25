#include "HeatVision.h"
#include "main.h"
#include "natives.h"
#include <cmath>

namespace HeatVision
{
    static bool g_active = false;
    static int g_tick = 0;

    void Toggle()
    {
        g_active = !g_active;
        g_tick = 0;
    }

    void Disable()
    {
        g_active = false;
        g_tick = 0;
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
            return;

        Vector3 start = CAM::GET_GAMEPLAY_CAM_COORD();
        Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);

        const float d2r = 0.01745329251994329577f;
        const float pitch = rot.x * d2r;
        const float yaw   = rot.z * d2r;

        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);

        Vector3 dir;
        dir.x = -sy * cp;
        dir.y =  cy * cp;
        dir.z =  sp;

        Vector3 end;
        end.x = start.x + dir.x * 60.0f;
        end.y = start.y + dir.y * 60.0f;
        end.z = start.z + dir.z * 60.0f;

        GRAPHICS::DRAW_LINE(
            start.x, start.y, start.z,
            end.x, end.y, end.z,
            255, 40, 10, 230);

        ++g_tick;
        if (g_tick >= 4)
        {
            g_tick = 0;

            Ped target = 0;
            if (PED::GET_CLOSEST_PED(
                    end.x, end.y, end.z,
                    3.0f,
                    TRUE, TRUE,
                    &target,
                    FALSE, FALSE,
                    4) &&
                target != 0 &&
                target != player &&
                ENTITY::DOES_ENTITY_EXIST(target))
            {
                int hp = ENTITY::GET_ENTITY_HEALTH(target);
                if (hp > 0)
                    ENTITY::SET_ENTITY_HEALTH(target, hp - 15);
            }
        }
    }
}
