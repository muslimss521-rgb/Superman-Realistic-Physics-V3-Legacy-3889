#include "Superman.h"
#include <algorithm>

void SupermanController::Tick(float dt)
{
    if (dt <= 0.0f)
        return;

    if (!enabled)
    {
        velocity = Vec3();
        return;
    }

    // Keep the internal flight model stable. The actual entity velocity is
    // controlled by UpdateFlight() in main.cpp; this state is used for
    // energy/speed calculations and future abilities.
    float speed = velocity.Length();

    if (speed > 0.001f)
    {
        const float dragCoefficient =
            abilities.state.boost ? 0.22f : 0.35f;

        Vec3 drag = Physics::Drag(
            velocity,
            1.225f,
            dragCoefficient,
            0.75f
        );

        velocity += (drag * (1.0f / std::max(1.0f, mass))) * dt;
    }

    const float hardLimit =
        abilities.state.boost ? boostSpeed : flightSpeed;

    float newSpeed = velocity.Length();

    if (newSpeed > hardLimit && newSpeed > 0.001f)
        velocity = velocity.Normalized() * hardLimit;
}
