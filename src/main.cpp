#define NOMINMAX
#include <windows.h>
#include <string>
#include <algorithm>
#include <cmath>

// Подключаем только базовые типы и вызывальщик нативов (это исключает циклические ошибки depth=1024)
#include "ScriptHookV/types.h"
#include "ScriptHookV/nativeCaller.h"

// Подключаем локальные модули физики
#include "Physics.h"
#include "Superman.h"

bool g_isFlying = false;
float g_currentSpeed = 0.0f;
float g_currentLean = 0.0f;

// Вызовы нативов через защищенный хэш-синтаксис (компилятор не сможет их "потерять")
void TriggerHeatVisionJulioNIB()
{
    // PLAYER::PLAYER_PED_ID()
    Ped playerPed = nativeCall<Ped>(0x43A66C31C68491C0);
    
    // CONTROLS::IS_CONTROL_PRESSED(0, 24)
    if (!nativeCall<BOOL>(0x1CE654FCD4D50B22, 0, 24)) return;

    // CAM::GET_GAMEPLAY_CAM_ROT(2)
    Vector3 camRot = nativeCall<Vector3>(0x837765A2533ECE65, 2);
    // CAM::GET_GAMEPLAY_CAM_COORD()
    Vector3 camCoord = nativeCall<Vector3>(0xFAAA931A783AEC66);
    
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

    // GRAPHICS::DRAW_LIGHT_WITH_RANGE(...)
    nativeCall<void>(0x66C4C50F33CED8E8, camCoord.x, camCoord.y, camCoord.z, 255, 0, 0, 30.0f, 15.0f);

    // GAMEPLAY::START_SHAPE_TEST_RAY(...)
    int raycast = nativeCall<int>(0x6A2924E9273DE2E6, camCoord.x, camCoord.y, camCoord.z, endCoords.x, endCoords.y, endCoords.z, -1, playerPed, 7);
    
    BOOL hit; Vector3 hitCoords; Vector3 surfaceNormal; Entity targetEntity;
    // GAMEPLAY::GET_SHAPE_TEST_RESULT(...)
    nativeCall<int>(0x3D6CDA4C5305EDE2, raycast, &hit, &hitCoords, &surfaceNormal, &targetEntity);

    // ENTITY::DOES_ENTITY_EXIST(targetEntity)
    if (hit && nativeCall<BOOL>(0x5A504DE5EDE36555, targetEntity))
    {
        // ENTITY::IS_ENTITY_A_PED(targetEntity)
        if (nativeCall<BOOL>(0x53351C66C3CD8132, targetEntity))
        {
            // PED::SET_PED_TO_RAGDOLL(...)
            nativeCall<void>(0xAE99CC83A3C088E2, targetEntity, 2000, 2000, 0, true, true, false);
            // ENTITY::APPLY_FORCE_TO_ENTITY(...)
            nativeCall<void>(0xC5F6E3E66F1CEDE4, targetEntity, 1, forwardVec.x * 60.0f, forwardVec.y * 60.0f, forwardVec.z * 35.0f, 0.0f, 0.0f, 0.0f, 0, false, true, true, true, true);
        }
        // ENTITY::IS_ENTITY_A_VEHICLE(targetEntity)
        else if (nativeCall<BOOL>(0x1253ECE4E50DE2E6, targetEntity))
        {
            // ENTITY::APPLY_FORCE_TO_ENTITY(...)
            nativeCall<void>(0xC5F6E3E66F1CEDE4, targetEntity, 1, forwardVec.x * 120.0f, forwardVec.y * 120.0f, forwardVec.z * 70.0f, 0.0f, 0.0f, 0.5f, 0, false, true, true, true, true);
        }
        
        // FIRE::START_ENTITY_FIRE(targetEntity)
        nativeCall<void>(0xF64E4D3E6C2E2EE6, targetEntity);
    }
}

void UpdateSupermanPhysics()
{
    // PLAYER::PLAYER_PED_ID()
    Ped playerPed = nativeCall<Ped>(0x43A66C31C68491C0);
    // ENTITY::IS_ENTITY_DEAD(playerPed)
    if (nativeCall<BOOL>(0x2D5C3E2C22D3E3E6, playerPed)) return;

    // CONTROLS::IS_CONTROL_JUST_PRESSED(0, 22)
    if (nativeCall<BOOL>(0x5F6D43E3882DEE22, 0, 22)) 
    {
        g_isFlying = !g_isFlying;
        if (!g_isFlying)
        {
            // AI::STOP_ANIM_TASK(...)
            nativeCall<void>(0x9924E2E6C35EDE44, playerPed, "anim@animations", "flight_loop", 3.0f);
            // ENTITY::SET_ENTITY_HAS_GRAVITY(playerPed, true)
            nativeCall<void>(0x21F3E3E66F2CEDE2, playerPed, true);
        }
        else
        {
            // ENTITY::SET_ENTITY_HAS_GRAVITY(playerPed, false)
            nativeCall<void>(0x21F3E3E66F2CEDE2, playerPed, false);
            // AI::TASK_PLAY_ANIM(...)
            nativeCall<void>(0x5A24E2E6C35EDE11, playerPed, "anim@animations", "flight_loop", 8.0f, -8.0f, -1, 9, 0.0f, false, false, false);
        }
    }

    if (!g_isFlying) {
        TriggerHeatVisionJulioNIB();
        return;
    }

    // CONTROLS::GET_CONTROL_NORMAL(...)
    float forwardInput = nativeCall<float>(0x32A66C31C68491A1, 0, 32) - nativeCall<float>(0x32A66C31C68491A1, 0, 33); 
    float turnInput = nativeCall<float>(0x32A66C31C68491A1, 0, 34) - nativeCall<float>(0x32A66C31C68491A1, 0, 35);    
    // CONTROLS::IS_CONTROL_PRESSED(...)
    bool isBoosting = nativeCall<BOOL>(0x1CE654FCD4D50B22, 0, 21); 

    // CAM::GET_GAMEPLAY_CAM_ROT(2)
    Vector3 camRot = nativeCall<Vector3>(0x837765A2533ECE65, 2);
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
    // GAMEPLAY::GET_FRAME_TIME()
    g_currentSpeed += acceleration * nativeCall<float>(0x15A66C31C68491F2);
    if (g_currentSpeed < 0.0f) g_currentSpeed = 0.0f;

    // ENTITY::SET_ENTITY_VELOCITY(...)
    nativeCall<void>(0x74F6E3E66F1CEDE2, playerPed, flightDirection.x * g_currentSpeed, flightDirection.y * g_currentSpeed, flightDirection.z * g_currentSpeed);

    if (g_currentSpeed >= SOUND_SPEED && isBoosting)
    {
        // ENTITY::GET_ENTITY_COORDS(...)
        Vector3 coords = nativeCall<Vector3>(0x3C5C3E2C22D3E3E2, playerPed, true);
        // FIRE::ADD_EXPLOSION(...)
        nativeCall<void>(0x4201E3E66F1CEDE2, coords.x, coords.y, coords.z, 34, 1.0f, true, false); 
        // GAMEPLAY::SHAKE_GAMEPLAY_CAM(...)
        nativeCall<void>(0x1201E3E66F1CEDE5, "LARGE_EXPLOSION_SHAKE", 1.2f);
    }

    float targetLean = -turnInput * 45.0f; 
    g_currentLean = g_currentLean + (targetLean - g_currentLean) * 0.1f;
    // ENTITY::SET_ENTITY_ROTATION(...)
    nativeCall<void>(0x82F6E3E66F1CEDE2, playerPed, camRot.x, 0.0f, camRot.z + g_currentLean, 2, true);

    TriggerHeatVisionJulioNIB();

    float normalizedSpeed = (g_currentSpeed / SOUND_SPEED);
    if (normalizedSpeed < 0.0f) normalizedSpeed = 0.0f;
    // ENTITY::GET_ENTITY_COORDS(...)
    Vector3 pCoords = nativeCall<Vector3>(0x3C5C3E2C22D3E3E2, playerPed, true);
    
    // GRAPHICS::DRAW_MARKER(...)
    nativeCall<void>(0x3201E3E66F1CEDE2, 1, pCoords.x, pCoords.y, pCoords.z - 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 2.0f, 0.5f, 255, 0, 0, (int)(normalizedSpeed * 255), false, true, 2, false, nullptr, nullptr, false);
}
