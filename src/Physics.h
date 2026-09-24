#pragma once
#define NOMINMAX
#include <windows.h>
#include <cmath>

struct DynamicVector3 {
    float x;
    float y;
    float z;
};

const float AIR_DENSITY = 1.225f;       
const float DRAG_COEFFICIENT = 0.25f;   
const float BODY_AREA = 0.4f;           
const float SUPER_MASS = 100.0f;        
const float SOUND_SPEED = 343.0f;       

inline float CalculateAirDrag(float currentSpeed) {
    float speedSquared = currentSpeed * currentSpeed;
    float currentCd = (currentSpeed >= SOUND_SPEED) ? 0.60f : DRAG_COEFFICIENT;
    return 0.5f * AIR_DENSITY * speedSquared * currentCd * BODY_AREA;
}
