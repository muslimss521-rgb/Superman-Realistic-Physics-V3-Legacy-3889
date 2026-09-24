#define NOMINMAX
#include <windows.h>
#include <string>
#include <algorithm>
#include <cmath>
#include "Superman.h"
#include "Abilities.h"

// Глобальные переменные физики полета (в стиле JulioNIB)
bool isFlying = false;
float currentSpeed = 0.0f;
const float maxSpeed = 343.0f;       // Скорость звука в м/с (1225 км/ч)
const float airDensity = 1.225f;    // Плотность воздуха кг/м3
const float dragCoefficient = 0.25f;// Базовый коэффициент лобового сопротивления тела
const float bodyArea = 0.4f;        // Площадь миделева сечения Супермена (м2)
float currentLean = 0.0f;

void UpdateSupermanPhysics()
{
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (ENTITY::IS_ENTITY_DEAD(playerPed)) return;

    // Включение/выключение полета на клавишу (например, Пробел в воздухе)
    if (PAD::IS_CONTROL_JUST_PRESSED(0, 22)) // Space
    {
        isFlying = !isFlying;
        if (!isFlying)
        {
            AI::STOP_ANIM_TASK(playerPed, "anim@animations", "flight_loop", 3.0f);
            ENTITY::SET_ENTITY_HAS_GRAVITY(playerPed, true);
        }
        else
        {
            ENTITY::SET_ENTITY_HAS_GRAVITY(playerPed, false);
            AI::TASK_PLAY_ANIM(playerPed, "anim@animations", "flight_loop", 8.0f, -8.0f, -1, 9, 0.0f, false, false, false);
        }
    }

    if (!isFlying) return;

    // Считывание управления
    float forwardInput = PAD::GET_CONTROL_NORMAL(0, 32) - PAD::GET_CONTROL_NORMAL(0, 33); // W / S
    float turnInput = PAD::GET_CONTROL_NORMAL(0, 34) - PAD::GET_CONTROL_NORMAL(0, 35);    // A / D
    bool isBoosting = PAD::GET_CONTROL_JUST_PRESSED(0, 21); // Left Shift

    Vector3 camDir = CAM::GET_GAMEPLAY_CAM_ROT(2);
    // Конвертация углов камеры в вектор направления
    float pitch = camDir.x * 0.0174532925f;
    float yaw = camDir.z * 0.0174532925f;
    Vector3 flightDirection = { -sin(yaw) * cos(pitch), cos(yaw) * cos(pitch), sin(pitch) };

    // Честный расчет ньютоновского сопротивления воздуха: F_drag = 0.5 * rho * v^2 * Cd * A
    float velocitySquared = currentSpeed * currentSpeed;
    float dragForce = 0.5f * airDensity * velocitySquared * dragCoefficient * bodyArea;
    
    // Динамический волновой кризис при переходе на сверхзвук (как у JulioNIB)
    float activeDragCoeff = (currentSpeed >= 343.0f) ? 0.6f : dragCoefficient;

    // Расчет тяги Супермена
    float thrust = 0.0f;
    if (forwardInput > 0.0f)
    {
        thrust = isBoosting ? 1500.0f : 400.0f;
    }

    // Изменение скорости на основе сил (Ускорение = Сила / Массу)
    float mass = 100.0f; // Условный вес Кларка
    float acceleration = (thrust - dragForce) / mass;
    currentSpeed += acceleration * MISC::GET_FRAME_TIME();
    if (currentSpeed < 0.0f) currentSpeed = 0.0f;

    // Применение физического вектора движения через нативную скорость движка RAGE
    Vector3 velocityVector = { flightDirection.x * currentSpeed, flightDirection.y * currentSpeed, flightDirection.z * currentSpeed };
    ENTITY::SET_ENTITY_VELOCITY(playerPed, velocityVector.x, velocityVector.y, velocityVector.z);

    // Эффект Звукового Удара (Sonic Boom)
    if (currentSpeed >= 343.0f && isBoosting)
    {
        Vector3 coords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
        FIRE::ADD_EXPLOSION(coords.x, coords.y, coords.z, 34, 0.0f, true, false, 1.0f, false); // EXPLOSION_SMOKE_TRAIL
        CAM::SHAKE_GAMEPLAY_CAM("LARGE_EXPLOSION_SHAKE", 1.2f);
    }

    // Реалистичный аэродинамический крен модели тела при маневрах
    float targetLean = -turnInput * 45.0f; 
    currentLean = currentLean + (targetLean - currentLean) * 0.1f;
    ENTITY::SET_ENTITY_ROTATION(playerPed, camDir.x, 0.0f, camDir.z + currentLean, 2, true);

    // Исправленный рендеринг маркера навигации (ровно 25 аргументов для ScriptHookV)
    const float intensity = (std::max)(0.0f, currentSpeed / maxSpeed);
    Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
    GRAPHICS::DRAW_MARKER(1, pCoords.x, pCoords.y, pCoords.z - 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.5f, 255, 0, 0, (int)(intensity * 255), false, true, 2, false, nullptr, nullptr, false);
}
