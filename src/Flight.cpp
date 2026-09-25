#include <cmath>

#include "ScriptHookV/natives.h"
#include "Flight.h"

namespace
{
    bool g_flying = false;

    const float GRAVITY = 9.81f;
    const float THRUST = 30.0f;
    const float BOOST_THRUST = 75.0f;
    const float MAX_SPEED = 55.0f;
    const float BOOST_MAX_SPEED = 120.0f;
    const float DRAG = 0.045f;
    const float HOVER_DAMP = 2.5f;
    const float TURN_SPEED = 5.0f;
    const float LEAN_ANGLE = 28.0f;

    float Clamp(float v, float lo, float hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    float Length(const Vector3& v)
    {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    Vector3 Normalize(const Vector3& v)
    {
        Vector3 r;
        float l = Length(v);

        if (l > 0.001f)
        {
            r.x = v.x / l;
            r.y = v.y / l;
            r.z = v.z / l;
        }
        else
        {
            r.x = 0.0f;
            r.y = 0.0f;
            r.z = 0.0f;
        }

        return r;
    }

    // GTA PAD controls. These are handled by the game/GameHub,
    // unlike Windows GetAsyncKeyState which may not receive F3/E.
    bool Down(int control)
    {
        return PAD::IS_CONTROL_PRESSED(0, control);
    }

    bool JustPressed(int control)
    {
        return PAD::IS_CONTROL_JUST_PRESSED(0, control);
    }

    Vector3 CameraForward()
    {
        Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);

        const float pi = 3.14159265359f;
        float pitch = rot.x * pi / 180.0f;
        float yaw = rot.z * pi / 180.0f;
        float cp = std::cos(pitch);

        Vector3 f;
        f.x = -std::sin(yaw) * cp;
        f.y =  std::cos(yaw) * cp;
        f.z = std::sin(pitch);

        return Normalize(f);
    }

    Vector3 CameraRight()
    {
        Vector3 f = CameraForward();

        Vector3 r;
        r.x = f.y;
        r.y = -f.x;
        r.z = 0.0f;

        return Normalize(r);
    }

    void SetFlight(Ped ped, bool enabled)
    {
        g_flying = enabled;

        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, !enabled);
        PED::SET_PED_CAN_RAGDOLL(ped, !enabled);

        if (!enabled)
        {
            Vector3 v = ENTITY::GET_ENTITY_VELOCITY(ped);
            v.z = 0.0f;
            ENTITY::SET_ENTITY_VELOCITY(ped, v.x, v.y, v.z);
        }
    }

    void RotateToVelocity(Ped ped, const Vector3& velocity, float dt, float side)
    {
        float speed = Length(velocity);
        if (speed < 3.0f)
            return;

        const float pi = 3.14159265359f;

        float yaw = std::atan2(velocity.y, velocity.x) * 180.0f / pi - 90.0f;
        float horizontal = std::sqrt(
            velocity.x * velocity.x +
            velocity.y * velocity.y
        );
        float pitch = std::atan2(velocity.z, horizontal) * 180.0f / pi;
        float roll = -side * LEAN_ANGLE;

        Vector3 current = ENTITY::GET_ENTITY_ROTATION(ped, 2);
        float t = Clamp(TURN_SPEED * dt, 0.0f, 1.0f);

        float dy = yaw - current.z;

        while (dy > 180.0f) dy -= 360.0f;
        while (dy < -180.0f) dy += 360.0f;

        ENTITY::SET_ENTITY_ROTATION(
            ped,
            current.x + (pitch - current.x) * t,
            current.y + (roll - current.y) * t,
            current.z + dy * t,
            2,
            TRUE
        );
    }
}

namespace Flight
{
    void Initialize()
    {
        g_flying = false;
    }

    void Update()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        // INPUT_CONTEXT (E on the normal GTA keyboard layout).
        // This goes through GTA's input system and is suitable for GameHub.
        if (JustPressed(51))
        {
            SetFlight(ped, !g_flying);
        }

        if (!g_flying)
            return;

        if (PED::IS_PED_IN_ANY_VEHICLE(ped, false))
            return;

        float dt = GAMEPLAY::GET_FRAME_TIME();
        dt = Clamp(dt, 0.001f, 0.05f);

        // GTA controls:
        // 32=W, 33=S, 34=A, 35=D
        // 22=Space, 36=Ctrl, 21=Shift
        float forwardInput = 0.0f;
        if (Down(32)) forwardInput += 1.0f;
        if (Down(33)) forwardInput -= 1.0f;

        float sideInput = 0.0f;
        if (Down(35)) sideInput += 1.0f;
        if (Down(34)) sideInput -= 1.0f;

        float verticalInput = 0.0f;
        if (Down(22)) verticalInput += 1.0f;
        if (Down(36)) verticalInput -= 1.0f;

        bool boost = Down(21);

        Vector3 velocity = ENTITY::GET_ENTITY_VELOCITY(ped);
        Vector3 forward = CameraForward();
        Vector3 right = CameraRight();

        float thrust = boost ? BOOST_THRUST : THRUST;

        // Newtonian acceleration.
        Vector3 acceleration;
        acceleration.x = 0.0f;
        acceleration.y = 0.0f;
        acceleration.z = -GRAVITY;

        acceleration.x += forward.x * thrust * forwardInput;
        acceleration.y += forward.y * thrust * forwardInput;
        acceleration.z += forward.z * thrust * forwardInput;

        const float sideThrust = 18.0f;
        acceleration.x += right.x * sideThrust * sideInput;
        acceleration.y += right.y * sideThrust * sideInput;

        const float verticalThrust = 28.0f;
        acceleration.z += verticalThrust * verticalInput;

        // Hover: counter gravity and damp vertical motion.
        if (verticalInput == 0.0f)
        {
            acceleration.z += GRAVITY;
            acceleration.z -= velocity.z * HOVER_DAMP;
        }

        // Quadratic aerodynamic drag.
        float speed = Length(velocity);
        if (speed > 0.01f)
        {
            float drag = DRAG * speed;
            acceleration.x -= velocity.x * drag;
            acceleration.y -= velocity.y * drag;
            acceleration.z -= velocity.z * drag;
        }

        velocity.x += acceleration.x * dt;
        velocity.y += acceleration.y * dt;
        velocity.z += acceleration.z * dt;

        float maxSpeed = boost ? BOOST_MAX_SPEED : MAX_SPEED;
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

        RotateToVelocity(ped, velocity, dt, sideInput);
    }
}
