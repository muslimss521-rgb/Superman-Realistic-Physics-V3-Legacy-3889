#include "Flight.h"
#include "../Input.h"
#include "main.h"
#include "natives.h"
#include <cmath>

namespace Flight
{
    static bool g_flying = false;
    static bool g_boost = false;
    static Vector3 g_velocity = { 0.0f, 0.0f, 0.0f };

    static float MoveTowards(float current, float target, float amount)
    {
        if (current < target)
        {
            current += amount;
            if (current > target) current = target;
        }
        else if (current > target)
        {
            current -= amount;
            if (current < target) current = target;
        }
        return current;
    }

    void Enable()
    {
        g_flying = true;
        g_boost = false;
        g_velocity.x = 0.0f;
        g_velocity.y = 0.0f;
        g_velocity.z = 0.0f;

        Ped ped = PLAYER::PLAYER_PED_ID();
        if (ENTITY::DOES_ENTITY_EXIST(ped))
        {
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, FALSE);
            ENTITY::SET_ENTITY_VELOCITY(ped, 0.0f, 0.0f, 0.0f);
        }
    }

    void Disable()
    {
        g_flying = false;
        g_boost = false;
        g_velocity.x = 0.0f;
        g_velocity.y = 0.0f;
        g_velocity.z = 0.0f;

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
        const float yaw = rot.z * d2r;
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

        // Julio-style tuning: documented public Suit Config values include
        // baseSprintSpeed=20 and jumpZSpeed=50. Exact closed-source flight
        // speed values are not assumed here.
        const float speed = g_boost ? 70.0f : 20.0f;
        const float verticalSpeed = g_boost ? 50.0f : 20.0f;
        const float acceleration = g_boost ? 120.0f : 55.0f;
        const float braking = 90.0f;
        const float dt = 0.035f;

        float tx = 0.0f;
        float ty = 0.0f;
        float tz = 0.0f;

        if (Input::Forward())
        {
            tx += forward.x * speed;
            ty += forward.y * speed;
            tz += forward.z * speed;
        }
        if (Input::Back())
        {
            tx -= forward.x * speed;
            ty -= forward.y * speed;
            tz -= forward.z * speed;
        }
        if (Input::Left())
        {
            tx -= right.x * speed;
            ty -= right.y * speed;
        }
        if (Input::Right())
        {
            tx += right.x * speed;
            ty += right.y * speed;
        }
        if (Input::Up())
            tz += verticalSpeed;
        if (Input::Down())
            tz -= verticalSpeed;

        g_velocity.x = MoveTowards(g_velocity.x, tx, acceleration * dt);
        g_velocity.y = MoveTowards(g_velocity.y, ty, acceleration * dt);
        g_velocity.z = MoveTowards(g_velocity.z, tz, acceleration * dt);

        if (!Input::Forward() && !Input::Back() &&
            !Input::Left() && !Input::Right())
        {
            g_velocity.x = MoveTowards(g_velocity.x, 0.0f, braking * dt);
            g_velocity.y = MoveTowards(g_velocity.y, 0.0f, braking * dt);
        }

        if (!Input::Up() && !Input::Down())
            g_velocity.z = MoveTowards(g_velocity.z, 0.0f, braking * dt);

        const float maxHorizontal = speed;
        const float horizontal = std::sqrt(
            g_velocity.x * g_velocity.x +
            g_velocity.y * g_velocity.y);

        if (horizontal > maxHorizontal && horizontal > 0.001f)
        {
            const float scale = maxHorizontal / horizontal;
            g_velocity.x *= scale;
            g_velocity.y *= scale;
        }

        if (g_velocity.z > 50.0f) g_velocity.z = 50.0f;
        if (g_velocity.z < -50.0f) g_velocity.z = -50.0f;

        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, FALSE);
        ENTITY::SET_ENTITY_VELOCITY(
            ped,
            g_velocity.x,
            g_velocity.y,
            g_velocity.z);

        if (Input::Forward() || Input::Back())
            ENTITY::SET_ENTITY_HEADING(ped, rot.z);
    }
}
