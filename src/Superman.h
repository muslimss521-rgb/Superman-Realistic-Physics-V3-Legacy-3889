#pragma once

#include "Physics.h"
#include "Abilities.h"

class SupermanController
{
public:
    bool enabled = false;

    float mass = 95.0f;

    float flightSpeed = 85.0f;
    float boostSpeed = 240.0f;

    float acceleration = 45.0f;
    float boostAcceleration = 110.0f;

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

    // Declaration only.
    // The implementation is in src/Superman.cpp.
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
