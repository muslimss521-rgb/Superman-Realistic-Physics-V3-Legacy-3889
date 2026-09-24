#pragma once
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cmath>

// Подключаем нативы ScriptHookV, чтобы компилятор знал пространства имен
#include "ScriptHookV/types.h"
#include "ScriptHookV/natives.h"
#include "ScriptHookV/nativeCaller.h"

inline void TriggerHeatVisionJulioNIB()
{
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (!PAD::IS_CONTROL_PRESSED(0, 24)) return; // Если ЛКМ не зажата — выходим

    Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
    Vector3 camCoord = CAM::GET_GAMEPLAY_CAM_COORD();
    
    float pitch = camRot.x * 0.0174532925f;
    float yaw = camRot.z * 0.0174532925f;
    
    // Вектор направления взгляда (forward vector)
    Vector3 forwardVec;
    forwardVec.x = -sin(yaw) * cos(pitch);
    forwardVec.y = cos(yaw) * cos(pitch);
    forwardVec.z = sin(pitch);
    
    Vector3 endCoords;
    endCoords.x = camCoord.x + forwardVec.x * 100.0f;
    endCoords.y = camCoord.y + forwardVec.y * 100.0f;
    endCoords.z = camCoord.z + forwardVec.z * 100.0f;

    // Отрисовка света из глаз
    GRAPHICS::DRAW_LIGHT_WITH_RANGE(camCoord.x, camCoord.y, camCoord.z, 255, 0, 0, 30.0f, 15.0f);

    // Определение точки физического контакта луча (ShapeTest)
    int raycast = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(camCoord.x, camCoord.y, camCoord.z, endCoords.x, endCoords.y, endCoords.z, -1, playerPed, 7);
    BOOL hit; Vector3 hitCoords; Vector3 surfaceNormal; Entity targetEntity;
    SHAPETEST::GET_SHAPE_TEST_RESULT(raycast, &hit, &hitCoords, &surfaceNormal, &targetEntity);

    if (hit && ENTITY::DOES_ENTITY_EXIST(targetEntity))
    {
        // Поведение в стиле JulioNIB: Физический импульс и Ragdoll
        if (ENTITY::IS_ENTITY_A_PED(targetEntity))
        {
            PED::SET_PED_TO_RAGDOLL(targetEntity, 2000, 2000, 0, true, true, false);
            ENTITY::APPLY_FORCE_TO_ENTITY(targetEntity, 1, forwardVec.x * 50.0f, forwardVec.y * 50.0f, forwardVec.z * 30.0f, 0.0f, 0.0f, 0.0f, 0, false, true, true, true, true);
        }
        else if (ENTITY::IS_ENTITY_A_VEHICLE(targetEntity))
        {
            ENTITY::APPLY_FORCE_TO_ENTITY(targetEntity, 1, forwardVec.x * 100.0f, forwardVec.y * 100.0f, forwardVec.z * 60.0f, 0.0f, 0.0f, 0.5f, 0, false, true, true, true, true);
        }
        
        FIRE::START_ENTITY_FIRE(targetEntity);
    }
}
