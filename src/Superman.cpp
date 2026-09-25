#include "Superman.h"
#include <algorithm>

void SupermanController::Tick(float dt)
{
    if (!enabled || dt <= 0.0f)
        return;

    float maxSpeed =
        abilities.state.boost
        ? boostSpeed
        : flightSpeed;

    if (maxSpeed < 1.0f)
        maxSpeed = 1.0f;

    float speed = velocity.Length();

    if (speed > maxSpeed)
        velocity = velocity.Normalized() * maxSpeed;

    // The actual GTA entity is driven by main.cpp. This controller
    // remains the source of physical parameters and kinetic energy.
}
