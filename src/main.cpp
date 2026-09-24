#define NOMINMAX
#include <windows.h>
#include <string>
#include <algorithm>
#include <cmath>

// Строгая последовательность включения SDK для исключения ошибок типов данных
#include "ScriptHookV/types.h"
#include "ScriptHookV/natives.h"

// Включение локальных модулей физики и способностей
#include "Physics.h"
#include "Abilities.h"
#include "Superman.h"

// Глобальное состояние симулятора
bool g_isFlying = false;
float g_currentSpeed = 0.0f;
float g_currentLean = 0.0f;

void UpdateSupermanPhysics()
{
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (ENTITY::IS_ENTITY_DEAD(playerPed)) return;

    // Взлет / Посадка по нажатию клавиши Space (Контрол 22)
    if (PAD::IS_CONTROL_JUST_PRESSED(0, 22)) 
    {
        g_isFlying = !g_isFlying;
        if (!g_isFlying)
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

    // Если не в режиме полета, обрабатываем только способности на земле
    if (!g_isFlying) {
        TriggerHeatVisionJulioNIB();
        return;
    }

    // Считывание команд перемещения со стандартных контроллеров RAGE
    float forwardInput = PAD::GET_CONTROL_NORMAL(0, 32) - PAD::GET_CONTROL_NORMAL(0, 33); // W / S
    float turnInput = PAD::GET_CONTROL_NORMAL(0, 34) - PAD::GET_CONTROL_NORMAL(0, 35);    // A / D
    bool isBoosting = PAD::IS_CONTROL_PRESSED(0, 21); // Зажатый Left Shift

    Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
    float pitch = camRot.x * 0.0174532925f;
    float yaw = camRot.z * 0.0174532925f;
    
    Vector3 flightDirection;
    flightDirection.x = -sin(yaw) * cos(pitch);
    flightDirection.y = cos(yaw) * cos(pitch);
    flightDirection.z = sin(pitch);

    // Расчет ньютоновского сопротивления среды из модуля Physics.h
    float dragForce = CalculateAirDrag(g_currentSpeed);

    // Вычисление силы тяги "внутреннего двигателя" Супермена
    float thrustForce = 0.0f;
    if (forwardInput > 0.0f)
    {
        thrustForce = isBoosting ? 2500.0f : 600.0f; // Сверхзвуковое ускорение vs Обычный крейсерский полет
    }

    // Изменение скорости: Ускорение = Сила / Масса (Закон Ньютона)
    float acceleration = (thrustForce - dragForce) / SUPER_MASS;
    g_currentSpeed += acceleration * MISC::GET_FRAME_TIME();
    if (g_currentSpeed < 0.0f) g_currentSpeed = 0.0f;

    // Применяем результирующий вектор физической скорости к сущности игрока
    ENTITY::SET_ENTITY_VELOCITY(
        playerPed, 
        flightDirection.x * g_currentSpeed, 
        flightDirection.y * g_currentSpeed, 
        flightDirection.z * g_currentSpeed
    );

    // Сверхзвуковой переход (Sonic Boom) с конусом ударной волны и кинематографичным сотрясением
    if (g_currentSpeed >= SOUND_SPEED && isBoosting)
    {
        Vector3 coords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
        FIRE::ADD_EXPLOSION(coords.x, coords.y, coords.z, 34, 0.0f, true, false, 1.0f, false); // Опечатка coords.coords.z исправлена!
        CAM::SHAKE_GAMEPLAY_CAM("LARGE_EXPLOSION_SHAKE", 1.2f);
    }

    // Реалистичный расчет крена тела при скоростных виражах мышью/клавишами
    float targetLean = -turnInput * 45.0f; 
    g_currentLean = g_currentLean + (targetLean - g_currentLean) * 0.1f;
    ENTITY::SET_ENTITY_ROTATION(playerPed, camRot.x, 0.0f, camRot.z + g_currentLean, 2, true);

    // Вызов лазера прямо во время полета
    TriggerHeatVisionJulioNIB();

    // Отрисовка маркера вектора тяги (строго валидные 25 параметров для компиляции)
    float normalizedSpeed = (std::max)(0.0f, g_currentSpeed / SOUND_SPEED);
    Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
    GRAPHICS::DRAW_MARKER(
        1, pCoords.x, pCoords.y, pCoords.z - 1.0f, 
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
        2.0f, 2.0f, 0.5f, 255, 0, 0, (int)(normalizedSpeed * 255), 
        false, true, 2, false, nullptr, nullptr, false
    );
}
