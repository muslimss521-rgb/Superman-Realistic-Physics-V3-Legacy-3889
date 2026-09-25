#include "Flight.h"
#include "../Input.h"
#include "main.h"
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
    }

    void SetBoost(bool enabled)
    {
        g_boost = enabled;
    }

    bool IsEnabled()
    {
        return g_flying;
    }

    void Update(Ped ped)
    {
        if (!g_flying || !ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);
        Vector3 forward = CAM::GET_GAMEPLAY_CAM_FORWARD_VECTOR();
        Vector3 right = CAM::GET_GAMEPLAY_CAM_RIGHT_VECTOR();

        float speed = g_boost ? 2.0f : 0.8f;

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

        ENTITY::SET_ENTITY_VELOCITY(ped, x, y, z);
        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, false);
    }
}
