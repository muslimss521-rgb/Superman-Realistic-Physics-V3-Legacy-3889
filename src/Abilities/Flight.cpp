#include "Flight.h"
#include "../Input.h"
#include "main.h"
#include "natives.h"
#include <cmath>

namespace Flight
{
    static bool g_flying = false;
    static bool g_boost = false;

    // Smoothed flight velocity. This avoids the "instant stop / instant start" feel.
    static float g_vx = 0.0f;
    static float g_vy = 0.0f;
    static float g_vz = 0.0f;

    static float MoveToward(float current, float target, float amount)
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
        g_vx = g_vy = g_vz = 0.0f;

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
        g_vx = g_vy = g_vz = 0.0f;

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

        Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);

        const float d2r = 0.01745329251994329577f;
        const float pitch = camRot.x * d2r;
        const float yaw   = camRot.z * d2r;

        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);

        // Camera forward vector: W/S follow exactly where the camera looks.
        Vector3 forward;
        forward.x = -sy * cp;
        forward.y =  cy * cp;
        forward.z =  sp;

        // Horizontal right vector.
        Vector3 right;
        right.x = cy;
        right.y = sy;
        right.z = 0.0f;

        const float maxSpeed = g_boost ? 75.0f : 22.0f;
        const float verticalSpeed = g_boost ? 50.0f : 14.0f;

        float targetX = 0.0f;
        float targetY = 0.0f;
        float targetZ = 0.0f;

        if (Input::Forward())
        {
            targetX += forward.x * maxSpeed;
            targetY += forward.y * maxSpeed;
            targetZ += forward.z * maxSpeed;
        }

        if (Input::Back())
        {
            targetX -= forward.x * maxSpeed;
            targetY -= forward.y * maxSpeed;
            targetZ -= forward.z * maxSpeed;
        }

        if (Input::Left())
        {
            targetX -= right.x * maxSpeed;
            targetY -= right.y * maxSpeed;
        }

        if (Input::Right())
        {
            targetX += right.x * maxSpeed;
            targetY += right.y * maxSpeed;
        }

        if (Input::Up())
            targetZ += verticalSpeed;

        if (Input::Down())
            targetZ -= verticalSpeed;

        // Smooth acceleration/deceleration.
        const float acceleration = g_boost ? 8.0f : 4.0f;

        g_vx = MoveToward(g_vx, targetX, acceleration);
        g_vy = MoveToward(g_vy, targetY, acceleration);
        g_vz = MoveToward(g_vz, targetZ, acceleration);

        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, FALSE);
        ENTITY::SET_ENTITY_VELOCITY(ped, g_vx, g_vy, g_vz);

        // Keep the character facing the camera horizontally.
        // Do NOT use SET_ENTITY_ANGULAR_VELOCITY here:
        // that native is not present in this ScriptHookV SDK.
        ENTITY::SET_ENTITY_HEADING(ped, camRot.z);
    }
}
