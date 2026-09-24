#pragma once
#define NOMINMAX
#include <windows.h>
#include <cmath>

// Не подключаем natives.h напрямую, чтобы избежать рекурсии компилятора
#include "ScriptHookV/types.h"
#include "ScriptHookV/nativeCaller.h"

inline void TriggerHeatVisionJulioNIB()
{
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    
    if (!CONTROLS::IS_CONTROL_PRESSED(0, 24)) return;

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

    int raycast = GAMEPLAY::START_SHAPE_TEST_RAY(
        camCoord.x, camCoord.y, camCoord.z, 
        endCoords.x, endCoords.y, endCoords.z, 
        -1, playerPed, 7
    );
    
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
