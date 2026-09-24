#pragma once

#include "Physics.h"
#include <algorithm>

struct AbilityState
{
    bool flight = false;
    bool boost = false;
    bool superSpeed = false;

    bool heatVision = false;
    bool freezeBreath = false;
    bool superBreath = false;

    bool bulletTime = false;
    bool xray = false;
    bool godMode = false;

    bool grabbing = false;
};

class AbilitySystem
{
public:

    AbilityState state;

    float heatPower = 1.0f;
    float freezeTemperature = -80.0f;
    float breathForce = 35.0f;

    float HeatEnergy(float deltaTime, float rangeFactor) const
    {
        if (deltaTime < 0.0f)
            deltaTime = 0.0f;

        if (rangeFactor < 0.0f)
            rangeFactor = 0.0f;

        return heatPower * deltaTime * rangeFactor;
    }

    float FreezeRate(float deltaTime, float temperatureFactor) const
    {
        if (deltaTime < 0.0f)
            deltaTime = 0.0f;

        if (temperatureFactor < 0.0f)
            temperatureFactor = 0.0f;

        return deltaTime * temperatureFactor;
    }

    float BreathForce(float distanceFactor) const
    {
        if (distanceFactor < 0.0f)
            distanceFactor = 0.0f;

        return breathForce * distanceFactor;
    }
};
