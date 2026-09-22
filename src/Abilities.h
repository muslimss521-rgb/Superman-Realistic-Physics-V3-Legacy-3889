#pragma once
#include "Physics.h"

struct AbilityState {
    bool flight=false, boost=false, superSpeed=false;
    bool heatVision=false, freezeBreath=false, superBreath=false;
    bool bulletTime=false, godMode=false;
    bool grabbing=false;
};

class AbilitySystem {
public:
    AbilityState state;
    float heatPower=1.0f;
    float freezeTemperature=-80.0f;
    float breathForce=35.0f;

    float HeatEnergy(float dt, float rangeFactor) const {
        return heatPower * std::max(0.0f, dt) * std::max(0.0f, rangeFactor);
    }

    float FreezeRate(float dt, float temperatureFactor) const {
        return std::max(0.0f, dt) * std::max(0.0f, temperatureFactor);
    }
};
