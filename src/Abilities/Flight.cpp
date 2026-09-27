#include "Flight.h"
#include "../Input.h"
#include "main.h"
#include "natives.h"
#include <cmath>

namespace Flight
{
    static bool g_flying = false;
    static bool g_boost = false;

    static float g_vx = 0.0f;
    static float g_vy = 0.0f;
    static float g_vz = 0.0f;

    static float Clamp(float v, float minV, float maxV)
    {
        return v < minV ? minV : (v > maxV ? maxV : v);
    }

    static float Approach(float current, float target, float amount)
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
        const float yaw = camRot.z * d2r;

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

        const bool forwardKey = Input::Forward();
        const bool backKey = Input::Back();
        const bool leftKey = Input::Left();
        const bool rightKey = Input::Right();
        const bool upKey = Input::Up();
        const bool downKey = Input::Down();

        float ix = 0.0f;
        float iy = 0.0f;
        float iz = 0.0f;

        if (forwardKey)
        {
            ix += forward.x;
            iy += forward.y;
            iz += forward.z;
        }

        if (backKey)
        {
            ix -= forward.x;
            iy -= forward.y;
            iz -= forward.z;
        }

        if (leftKey)
        {
            ix -= right.x;
            iy -= right.y;
        }

        if (rightKey)
        {
            ix += right.x;
            iy += right.y;
        }

        if (upKey)   iz += 1.0f;
        if (downKey) iz -= 1.0f;

        const float inputLen = std::sqrt(ix * ix + iy * iy + iz * iz);
        if (inputLen > 1.0f)
        {
            ix /= inputLen;
            iy /= inputLen;
            iz /= inputLen;
        }

        const float maxSpeed = g_boost ? 95.0f : 28.0f;
        const float acceleration = g_boost ? 8.5f : 4.5f;
        const float braking = g_boost ? 5.5f : 3.5f;

        const float targetX = ix * maxSpeed;
        const float targetY = iy * maxSpeed;
        const float targetZ = iz * (g_boost ? 65.0f : 22.0f);

        const float step = (inputLen > 0.01f) ? acceleration : braking;

        g_vx = Approach(g_vx, targetX, step);
        g_vy = Approach(g_vy, targetY, step);
        g_vz = Approach(g_vz, targetZ, step);

        // No keys = stable hover. Gradually brake instead of snapping.
        if (inputLen <= 0.01f)
        {
            g_vx = Approach(g_vx, 0.0f, braking);
            g_vy = Approach(g_vy, 0.0f, braking);
            g_vz = Approach(g_vz, 0.0f, braking);
        }

        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, FALSE);
        ENTITY::SET_ENTITY_VELOCITY(ped, g_vx, g_vy, g_vz);

        // Face horizontally toward the camera.
        ENTITY::SET_ENTITY_HEADING(ped, camRot.z);

        // Keep the ped physically stable while airborne.
        ENTITY::SET_ENTITY_ANGULAR_VELOCITY(ped, 0.0f, 0.0f, 0.0f);
    }
}
