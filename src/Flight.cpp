#include <windows.h>
#include <cmath>

#include "ScriptHookV/natives.h"
#include "Flight.h"

namespace
{
    bool g_flying = false;
    bool g_lastF3 = false;

    // Newton-style flight parameters.
    const float GRAVITY = 9.81f;
    const float THRUST = 30.0f;
    const float BOOST_THRUST = 75.0f;
    const float MAX_SPEED = 55.0f;
    const float BOOST_MAX_SPEED = 120.0f;

    const float DRAG = 0.045f;
    const float HOVER_VERTICAL_DAMP = 2.5f;
    const float TURN_SPEED = 5.0f;
    const float LEAN_ANGLE = 28.0f;

    float Clamp(float value, float minimum, float maximum)
    {
        if (value < minimum) return minimum;
        if (value > maximum) return maximum;
        return value;
    }

    float Length(const Vector3& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    Vector3 Normalize(const Vector3& v)
    {
        Vector3 result;
        float len = Length(v);

        if (len > 0.001f)
        {
            result.x = v.x / len;
            result.y = v.y / len;
            result.z = v.z / len;
        }
        else
        {
            result.x = 0.0f;
            result.y = 0.0f;
            result.z = 0.0f;
        }

        return result;
    }

    Vector3 CameraForward()
    {
        Vector3 rotation = CAM::GET_GAMEPLAY_CAM_ROT(2);

        const float pi = 3.14159265359f;
        float pitch = rotation.x * pi / 180.0f;
        float yaw = rotation.z * pi / 180.0f;

        float cp = std::cos(pitch);

        Vector3 forward;
        forward.x = -std::sin(yaw) * cp;
        forward.y =  std::cos(yaw) * cp;
        forward.z =  std::sin(pitch);

        return Normalize(forward);
    }

    Vector3 CameraRight()
    {
        Vector3 forward = CameraForward();

        Vector3 right;
        right.x = forward.y;
        right.y = -forward.x;
        right.z = 0.0f;

        return Normalize(right);
    }

    bool KeyDown(int vk)
    {
        return (GetAsyncKeyState(vk) & 0x8000) != 0;
    }

    bool PressedOnce(int vk, bool& previous)
    {
        bool current = KeyDown(vk);
        bool pressed = current && !previous;
        previous = current;
        return pressed;
    }

    void SetFlightState(Ped ped, bool enabled)
    {
        g_flying = enabled;

        if (enabled)
        {
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, false);
            PED::SET_PED_CAN_RAGDOLL(ped, false);
        }
        else
        {
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, true);
            PED::SET_PED_CAN_RAGDOLL(ped, true);
        }
    }

    void UpdateRotation(Ped ped, const Vector3& velocity, float dt, float inputX)
    {
        float speed = Length(velocity);
        if (speed < 3.0f)
            return;

        const float pi = 3.14159265359f;

        float yaw = std::atan2(velocity.y, velocity.x) * 180.0f / pi - 90.0f;
        float horizontal = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        float pitch = std::atan2(velocity.z, horizontal) * 180.0f / pi;

        float roll = -inputX * LEAN_ANGLE;

        Vector3 current = ENTITY::GET_ENTITY_ROTATION(ped, 2);

        float factor = Clamp(TURN_SPEED * dt, 0.0f, 1.0f);

        float targetYaw = yaw;
        float deltaYaw = targetYaw - current.z;

        while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
        while (deltaYaw < -180.0f) deltaYaw += 360.0f;

        float newYaw = current.z + deltaYaw * factor;
        float newPitch = current.x + (pitch - current.x) * factor;
        float newRoll = current.y + (roll - current.y) * factor;

        ENTITY::SET_ENTITY_ROTATION(ped, newPitch, newRoll, newYaw, 2, TRUE);
    }
}

namespace Flight
{
    void Initialize()
    {
        g_flying = false;
        g_lastF3 = false;
    }

    void Update()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        if (PressedOnce(VK_F3, g_lastF3))
        {
            SetFlightState(ped, !g_flying);

            if (!g_flying)
            {
                Vector3 v = ENTITY::GET_ENTITY_VELOCITY(ped);

                // Keep horizontal inertia, but remove dangerous vertical speed on landing.
                v.z = 0.0f;
                ENTITY::SET_ENTITY_VELOCITY(ped, v.x, v.y, v.z);
            }
        }

        if (!g_flying)
            return;

        if (PED::IS_PED_IN_ANY_VEHICLE(ped, false))
            return;

        float dt = GAMEPLAY::GET_FRAME_TIME();
        dt = Clamp(dt, 0.001f, 0.05f);

        // W/S = forward/back, A/D = lateral steering.
        float forwardInput = 0.0f;
        if (KeyDown('W')) forwardInput += 1.0f;
        if (KeyDown('S')) forwardInput -= 1.0f;

        float sideInput = 0.0f;
        if (KeyDown('D')) sideInput += 1.0f;
        if (KeyDown('A')) sideInput -= 1.0f;

        float verticalInput = 0.0f;
        if (KeyDown(VK_SPACE)) verticalInput += 1.0f;
        if (KeyDown(VK_CONTROL)) verticalInput -= 1.0f;

        bool boosting = KeyDown(VK_LSHIFT) || KeyDown(VK_RSHIFT);

        Vector3 velocity = ENTITY::GET_ENTITY_VELOCITY(ped);

        Vector3 forward = CameraForward();
        Vector3 right = CameraRight();

        // Newton's second law: acceleration from thrust.
        float thrust = boosting ? BOOST_THRUST : THRUST;

        Vector3 acceleration;
        acceleration.x = 0.0f;
        acceleration.y = 0.0f;
        acceleration.z = -GRAVITY;

        // Flight thrust follows the camera direction.
        acceleration.x += forward.x * thrust * forwardInput;
        acceleration.y += forward.y * thrust * forwardInput;
        acceleration.z += forward.z * thrust * forwardInput;

        // Lateral control.
        const float sideThrust = 18.0f;
        acceleration.x += right.x * sideThrust * sideInput;
        acceleration.y += right.y * sideThrust * sideInput;

        // Explicit vertical thrust for takeoff/landing control.
        const float verticalThrust = 28.0f;
        acceleration.z += verticalThrust * verticalInput;

        // Hover stabilization: counter gravity and damp vertical motion.
        if (verticalInput == 0.0f)
        {
            acceleration.z += GRAVITY;
            acceleration.z -= velocity.z * HOVER_VERTICAL_DAMP;
        }

        // Quadratic aerodynamic drag.
        float speed = Length(velocity);
        if (speed > 0.01f)
        {
            float dragScale = DRAG * speed;

            acceleration.x -= velocity.x * dragScale;
            acceleration.y -= velocity.y * dragScale;
            acceleration.z -= velocity.z * dragScale;
        }

        // Integrate v = v + a * dt.
        velocity.x += acceleration.x * dt;
        velocity.y += acceleration.y * dt;
        velocity.z += acceleration.z * dt;

        float maxSpeed = boosting ? BOOST_MAX_SPEED : MAX_SPEED;
        float newSpeed = Length(velocity);

        if (newSpeed > maxSpeed)
        {
            Vector3 n = Normalize(velocity);
            velocity.x = n.x * maxSpeed;
            velocity.y = n.y * maxSpeed;
            velocity.z = n.z * maxSpeed;
        }

        ENTITY::SET_ENTITY_VELOCITY(
            ped,
            velocity.x,
            velocity.y,
            velocity.z
        );

        UpdateRotation(ped, velocity, dt, sideInput);
    }
}
