#pragma once
#include <windows.h>
#include "nativeCaller.h"

inline void TriggerHeatVisionJulioNIB()
{
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (!PAD::IS_CONTROL_PRESSED(0, 24)) return; // ЛКМ зажата

    Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
    Vector3 camCoord = CAM::GET_GAMEPLAY_CAM_COORD();
    
    float pitch = camRot.x * 0.0174532925f;
    float yaw = camRot.z * 0.0174532925f;
    Vector3 forwardVec = { -sin(yaw) * cos(pitch), cos(yaw) * cos(pitch), sin(pitch) };
    
    Vector3 endCoords = { camCoord.x + forwardVec.x * 100.0f, camCoord.y + forwardVec.y * 100.0f, camCoord.z + forwardVec.z * 100.0f };

    // Корректный нативный вызов DRAW_LIGHT_WITH_RANGE с физической интенсивностью
    GRAPHICS::DRAW_LIGHT_WITH_RANGE(camCoord.x, camCoord.y, camCoord.z, 255, 0, 0, 30.0f, 15.0f);

    // Определение точки физического контакта луча (ShapeTest)
    int raycast = SHAPETEST::START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(camCoord.x, camCoord.y, camCoord.z, endCoords.x, endCoords.y, endCoords.z, -1, playerPed, 7);
    BOOL hit; Vector3 hitCoords; Vector3 surfaceNormal; Entity targetEntity;
    SHAPETEST::GET_SHAPE_TEST_RESULT(raycast, &hit, &hitCoords, &surfaceNormal, &targetEntity);

    if (hit && ENTITY::DOES_ENTITY_EXIST(targetEntity))
    {
        // Если попали в NPC: отправляем его в глубокий Ragdoll-полет силой фотонного давления луча
        if (ENTITY::IS_ENTITY_A_PED(targetEntity))
        {
            PED::SET_PED_TO_RAGDOLL(targetEntity, 2000, 2000, 0, true, true, false);
            ENTITY::APPLY_FORCE_TO_ENTITY(targetEntity, 1, forwardVec.x * 50.0f, forwardVec.y * 50.0f, forwardVec.z * 30.0f, 0.0f, 0.0f, 0.0f, 0, false, true, true, true, true);
        }
        // Если попали в автомобиль: переворачиваем и сминаем кузов направленным импульсом
        else if (ENTITY::IS_ENTITY_A_VEHICLE(targetEntity))
        {
            ENTITY::APPLY_FORCE_TO_ENTITY(targetEntity, 1, forwardVec.x * 100.0f, forwardVec.y * 100.0f, forwardVec.z * 60.0f, 0.0f, 0.0f, 0.5f, 0, false, true, true, true, true);
        }
        
        FIRE::START_ENTITY_FIRE(targetEntity);
    }
}
