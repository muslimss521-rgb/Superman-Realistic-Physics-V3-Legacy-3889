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
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, true);
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

        // The SDK used by this project does not expose
        // GET_GAMEPLAY_CAM_FORWARD_VECTOR / RIGHT_VECTOR.
        // Calculate both vectors from the available camera rotation native.
        Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);

        const float degToRad = 0.017453292519943295769f;
        const float pitch = rot.x * degToRad;
        const float yaw   = rot.z * degToRad;

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

        const float speed = g_boost ? 3.5f : 1.0f;

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        if (Input::Forward())
        {
            x += forward.x * speed;
            y += forward.y * speed;
            z += forward.z * speed;
        }

        if (Input::Back())
        {
            x -= forward.x * speed;
            y -= forward.y * speed;
            z -= forward.z * speed;
        }

        if (Input::Left())
        {
            x -= right.x * speed;
            y -= right.y * speed;
        }

        if (Input::Right())
        {
            x += right.x * speed;
            y += right.y * speed;
        }

        if (Input::Up())
            z += speed;

        if (Input::Down())
            z -= speed;

        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, false);
        ENTITY::SET_ENTITY_VELOCITY(ped, x, y, z);
    }
}
