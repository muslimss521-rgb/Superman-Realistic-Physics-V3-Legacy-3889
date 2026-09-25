#include "Flight.h"
#include "../Input.h"
#include "../main.h"
#include <cmath>

namespace
{
    bool g_enabled = false;
    float g_velocity = 0.0f;

    float Clamp(float value, float minValue, float maxValue)
    {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }
}

namespace Flight
{
    void Initialize()
    {
        g_enabled = false;
        g_velocity = 0.0f;
    }

    void Enable()
    {
        g_enabled = true;
        g_velocity = 0.0f;

        Ped ped = PLAYER::PLAYER_PED_ID();
        if (ENTITY::DOES_ENTITY_EXIST(ped))
        {
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, false);
            PED::SET_PED_CAN_RAGDOLL(ped, false);
        }
    }

    void Disable()
    {
        g_enabled = false;
        g_velocity = 0.0f;

        Ped ped = PLAYER::PLAYER_PED_ID();
        if (ENTITY::DOES_ENTITY_EXIST(ped))
        {
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, true);
            PED::SET_PED_CAN_RAGDOLL(ped, true);
        }
    }

    void Update(bool boost)
    {
        if (!g_enabled)
            return;

        Ped ped = PLAYER::PLAYER_PED_ID();
        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        Vector3 forward = CAM::GET_GAMEPLAY_CAM_FORWARD_VECTOR();
        Vector3 right = CAM::GET_GAMEPLAY_CAM_RIGHT_VECTOR();

        const float acceleration = boost ? 110.0f : 45.0f;
        const float maxSpeed = boost ? 160.0f : 75.0f;
        const float vertical = 35.0f;
        const float strafe = 45.0f;

        if (Input::Down(Input::Key::Forward))
            g_velocity += acceleration * 0.016f;
        else if (Input::Down(Input::Key::Back))
            g_velocity -= acceleration * 0.016f;
        else
            g_velocity *= 0.94f;

        g_velocity = Clamp(g_velocity, -maxSpeed * 0.45f, maxSpeed);

        Vector3 velocity{};
        velocity.x = forward.x * g_velocity;
        velocity.y = forward.y * g_velocity;
        velocity.z = forward.z * g_velocity;

        if (Input::Down(Input::Key::Left))
        {
            velocity.x -= right.x * strafe;
            velocity.y -= right.y * strafe;
        }

        if (Input::Down(Input::Key::Right))
        {
            velocity.x += right.x * strafe;
            velocity.y += right.y * strafe;
        }

        if (Input::Down(Input::Key::Up))
            velocity.z += vertical;

        if (Input::Down(Input::Key::Down))
            velocity.z -= vertical;

        if (Input::Down(Input::Key::Emergency))
        {
            Disable();
            return;
        }

        ENTITY::SET_ENTITY_VELOCITY(ped, velocity.x, velocity.y, velocity.z);
        ENTITY::SET_ENTITY_ROTATION(
            ped,
            0.0f,
            0.0f,
            CAM::GET_GAMEPLAY_CAM_RELATIVE_HEADING(),
            2,
            true
        );
    }

    bool IsEnabled()
    {
        return g_enabled;
    }
}
