#include "Superman.h"
#include <algorithm>

void SupermanController::Tick(float dt)
{
    if (!enabled || dt <= 0.0f)
        return;

    float maxSpeed = abilities.state.boost ? boostSpeed : flightSpeed;
    float accel = abilities.state.boost ? boostAcceleration : acceleration;

    float speed = velocity.Length();

    if (speed < maxSpeed)
    {
        float next = std::min(maxSpeed, speed + accel * dt);

        if (speed > 0.001f)
            velocity = velocity.Normalized() * next;
        else
            velocity = Vec3();
    }

    Vec3 drag = Physics::Drag(
        velocity,
        1.225f,
        0.35f,
        0.75f
    );

    Vec3 accelerationFromDrag = drag * (1.0f / mass);
    velocity += accelerationFromDrag * dt;
}
