#include "Flight.h"
#include "../Input.h"
#include "main.h"
#include "natives.h"
#include <cmath>

namespace Flight
{
    static bool g_flying = false;
    static bool g_boost = false;

    void Enable()
    {
        g_flying = true;
    }

    void Disable()
    {
        g_flying = false;
        g_boost = false;

        Ped ped = PLAYER::PLAYER_PED_ID();
        if (ENTITY::DOES_ENTITY_EXIST(ped))
        {
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, TRUE);
            ENTITY::SET_ENTITY_VELOCITY(ped, 0.0f, 0.0f, 0.0f);
        }
    }

    void SetBoost(bool enabled)
    {
        g_boost = enabled;
    }

    bool IsEnabled()
    {
        return g_flying;
    }

    void Update()
    {
        if (!g_flying)
            return;

        Ped ped = PLAYER::PLAYER_PED_ID();
        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);

        const float d2r = 0.01745329251994329577f;
        const float pitch = rot.x * d2r;
        const float yaw   = rot.z * d2r;

        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);

        Vector3 forward;
        forward.x = -sy * cp;
        forward.y =  cy * cp;
        forward.z =  sp;

        Vector3 right;
        right.x = cy;
        right.y = sy;
        right.z = 0.0f;

        const float speed = g_boost ? 4.0f : 1.25f;

        float vx = 0.0f;
        float vy = 0.0f;
        float vz = 0.0f;

        if (Input::Forward())
        {
            vx += forward.x * speed;
            vy += forward.y * speed;
            vz += forward.z * speed;
        }

        if (Input::Back())
        {
            vx -= forward.x * speed;
            vy -= forward.y * speed;
            vz -= forward.z * speed;
        }

        if (Input::Left())
        {
            vx -= right.x * speed;
            vy -= right.y * speed;
        }

        if (Input::Right())
        {
            vx += right.x * speed;
            vy += right.y * speed;
        }

        if (Input::Up())
            vz += speed;

        if (Input::Down())
            vz -= speed;

        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, FALSE);
        ENTITY::SET_ENTITY_VELOCITY(ped, vx, vy, vz);

        // Turn the character toward the camera while flying.
        ENTITY::SET_ENTITY_HEADING(ped, rot.z);
    }
}
