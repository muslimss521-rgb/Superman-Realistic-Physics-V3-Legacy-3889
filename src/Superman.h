#pragma once
#define NOMINMAX
#include <windows.h>

// Подключаем SDK строго здесь один раз на весь проект
#include "ScriptHookV/types.h"
#include "ScriptHookV/natives.h"

class Superman 
{
private:
    bool isFlyingEnabled;
    float currentFlightSpeed;
    float bodyRollAngle;

public:
    Superman() : isFlyingEnabled(false), currentFlightSpeed(0.0f), bodyRollAngle(0.0f) {}
    
    void Initialize();
    void UpdateFlightState();
    void ProcessSuperAbilities();
};
