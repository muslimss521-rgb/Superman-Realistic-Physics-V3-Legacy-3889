#pragma once
#define NOMINMAX
#include <windows.h>
#include <cmath>

// Структура для работы с трехмерными векторами физики
struct DynamicVector3 {
    float x;
    float y;
    float z;
};

// Параметры реалистичной атмосферной симуляции
const float AIR_DENSITY = 1.225f;       // Плотность воздуха на уровне моря (кг/м³)
const float DRAG_COEFFICIENT = 0.25f;   // Базовый коэффициент обтекаемости тела Супермена
const float BODY_AREA = 0.4f;           // Миделево сечение человека (м²)
const float SUPER_MASS = 100.0f;        // Физическая масса персонажа (кг)
const float SOUND_SPEED = 343.0f;       // Скорость звука (м/с)

// Вычисление силы лобового сопротивления по закону Ньютона: F = 0.5 * rho * v^2 * Cd * A
inline float CalculateAirDrag(float currentSpeed) {
    float speedSquared = currentSpeed * currentSpeed;
    // На сверхзвуке коэффициент сопротивления резко растет (волновой кризис)
    float currentCd = (currentSpeed >= SOUND_SPEED) ? 0.60f : DRAG_COEFFICIENT;
    return 0.5f * AIR_DENSITY * speedSquared * currentCd * BODY_AREA;
}
