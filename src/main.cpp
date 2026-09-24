#define NOMINMAX
#include <windows.h>
#include <string>
#include <algorithm>
#include <cmath>

// 1. ПОДКЛЮЧАЕМ SDK СТРОГО ТУТ (Компилятор сразу увидит все типы движка)
#include "ScriptHookV/types.h"
#include "ScriptHookV/natives.h"

// 2. ПОДКЛЮЧАЕМ ЛОКАЛЬНЫЕ КОНФИГУРАЦИИ
#include "Physics.h"
#include "Superman.h"

// Состояния симулятора
bool g_isFlying = false;
float g_currentSpeed = 0.0f;
float g_currentLean = 0.0f;

// Логика боевого лазера в стиле JulioNIB, перенесенная сюда во избежание рекурсии заголовков
void TriggerHeatVisionJulioNIB()
{
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (!CONTROLS::IS_CONTROL_PRESSED(0, 24)) return; // ЛКМ

    Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
    Vector3 camCoord = CAM::GET_GAMEPLAY_CAM_COORD();
    
    float pitch = camRot.x * 0.0174532925f;
    float yaw = camRot.z * 0.0174532925f;
    
    Vector3 forwardVec;
    forwardVec.x = -sin(yaw) * cos(pitch);
    forwardVec.y = cos(yaw) * cos(pitch);
    forwardVec.z = sin(pitch);
    
    Vector3 endCoords;
    endCoords.x = camCoord.x + forwardVec.x * 100.0f;
    endCoords.y = camCoord.y + forwardVec.y * 100.0f;
    endCoords.z = camCoord.z + forwardVec.z * 100.0f;

    GRAPHICS::DRAW_LIGHT_WITH_RANGE(camCoord.x, camCoord.y, camCoord.z, 255, 0, 0, 30.0f, 15.0f);

    int raycast = GAMEPLAY::START_SHAPE_TEST_RAY(camCoord.x, camCoord.y, camCoord.z, endCoords.x, endCoords.y, endCoords.z, -1, playerPed, 7);
    BOOL hit; Vector3 hitCoords; Vector3 surfaceNormal; Entity targetEntity;
    GAMEPLAY::GET_SHAPE_TEST_RESULT(raycast, &hit, &hitCoords, &surfaceNormal, &targetEntity);

    if (hit && ENTITY::DOES_ENTITY_EXIST(targetEntity))
    {
        if (ENTITY::IS_ENTITY_A_PED(targetEntity))
        {
            PED::SET_PED_TO_RAGDOLL(targetEntity, 2000, 2000, 0, true, true, false);
            ENTITY::APPLY_FORCE_TO_ENTITY(targetEntity, 1, forwardVec.x * 60.0f, forwardVec.y * 60.0f, forwardVec.z * 35.0f, 0.0f, 0.0f, 0.0f, 0, false, true, true, true, true);
        }
        else if (ENTITY::IS_ENTITY_A_VEHICLE(targetEntity))
        {
            ENTITY::APPLY_FORCE_TO_ENTITY(targetEntity, 1, forwardVec.x * 120.0f, forwardVec.y * 120.0f, forwardVec.z * 70.0f, 0.0f, 0.0f, 0.5f, 0, false, true, true, true, true);
        }
        FIRE::START_ENTITY_FIRE(targetEntity);
    }
}

void UpdateSupermanPhysics()
{
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (ENTITY::IS_ENTITY_DEAD(playerPed)) return;

    if (CONTROLS::IS_CONTROL_JUST_PRESSED(0, 22)) // Пробел
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

    if (!g_isFlying) {
        TriggerHeatVisionJulioNIB();
        return;
    }

    float forwardInput = CONTROLS::GET_CONTROL_NORMAL(0, 32) - CONTROLS::GET_CONTROL_NORMAL(0, 33); 
    float turnInput = CONTROLS::GET_CONTROL_NORMAL(0, 34) - CONTROLS::GET_CONTROL_NORMAL(0, 35);    
    bool isBoosting = CONTROLS::IS_CONTROL_PRESSED(0, 21); // Shift

    Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
    float pitch = camRot.x * 0.0174532925f;
    float yaw = camRot.z * 0.0174532925f;
    
    Vector3 flightDirection;
    flightDirection.x = -sin(yaw) * cos(pitch);
    flightDirection.y = cos(yaw) * cos(pitch);
    flightDirection.z = sin(pitch);

    float dragForce = CalculateAirDrag(g_currentSpeed);

    float thrustForce = 0.0f;
    if (forwardInput > 0.0f)
    {
        thrustForce = isBoosting ? 2500.0f : 600.0f; 
    }

    float acceleration = (thrustForce - dragForce) / SUPER_MASS;
    g_currentSpeed += acceleration * GAMEPLAY::GET_FRAME_TIME();
    if (g_currentSpeed < 0.0f) g_currentSpeed = 0.0f;

    ENTITY::SET_ENTITY_VELOCITY(playerPed, flightDirection.x * g_currentSpeed, flightDirection.y * g_currentSpeed, flightDirection.z * g_currentSpeed);

    if (g_currentSpeed >= SOUND_SPEED && isBoosting)
    {
        Vector3 coords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
        FIRE::ADD_EXPLOSION(coords.x, coords.y, coords.z, 34, 1.0f, true, false); 
        GAMEPLAY::SHAKE_GAMEPLAY_CAM("LARGE_EXPLOSION_SHAKE", 1.2f);
    }

    float targetLean = -turnInput * 45.0f; 
    g_currentLean = g_currentLean + (targetLean - g_currentLean) * 0.1f;
    ENTITY::SET_ENTITY_ROTATION(playerPed, camRot.x, 0.0f, camRot.z + g_currentLean, 2, true);

    TriggerHeatVisionJulioNIB();

    float normalizedSpeed = (std::max)(0.0f, g_currentSpeed / SOUND_SPEED);
    Vector3 pCoords = ENTITY::GET_ENTITY_COORDS(playerPed, true);
    GRAPHICS::DRAW_MARKER(1, pCoords.x, pCoords.y, pCoords.z - 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.5f, 255, 0, 0, (int)(normalizedSpeed * 255), false, true, 2, false, nullptr, nullptr, false);
}
