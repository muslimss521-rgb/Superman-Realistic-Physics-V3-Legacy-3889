#pragma once

#include "Physics.h"
#include "Abilities.h"

class SupermanController
{
public:
    bool enabled = false;

    float mass = 95.0f;

    // Ported from the supplied Unity RealisticSupermanFlight settings.
    float flightForce = 30.0f;
    float flightSpeed = 50.0f;
    float boostMultiplier = 2.5f;
    float boostSpeed = 125.0f;

    float turnSpeed = 3.0f;
    float airDrag = 1.0f;
    float stopDrag = 5.0f;

    float leanAmount = 35.0f;
    float leanSpeed = 5.0f;

    float acceleration = 30.0f;
    float boostAcceleration = 75.0f;

    Vec3 velocity{};
    Vec3 angularVelocity{};

    AbilitySystem abilities;

    void Reset()
    {
        enabled = false;

        velocity = {};
        angularVelocity = {};

        abilities.state = {};
    }

    void Tick(float dt);

    float CurrentSpeed() const
    {
        return velocity.Length();
    }

    float CurrentKineticEnergy() const
    {
        return Physics::KineticEnergy(
            mass,
            CurrentSpeed()
        );
    }
};
