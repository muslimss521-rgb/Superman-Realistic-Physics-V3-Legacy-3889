#define NOMINMAX
#include <windows.h>
#include <string>
#include <algorithm>
#include <cmath>

// Подключаем только базовые типы и кастомный вызов через ScriptHookV без шаблонов
#include "ScriptHookV/types.h"
#include "ScriptHookV/nativeCaller.h"

// Локальные конфигурации физики
#include "Physics.h"
#include "Superman.h"

bool g_isFlying = false;
float g_currentSpeed = 0.0f;
float g_currentLean = 0.0f;

// Старый синтаксис ScriptHookV для вызова нативов через стек (работает в 100% версий SDK)
void TriggerHeatVisionJulioNIB()
{
    // PLAYER::PLAYER_PED_ID()
    nativeInit(0x43A66C31C68491C0);
    Ped playerPed = *nativeCall();

    // CONTROLS::IS_CONTROL_PRESSED(0, 24)
    nativeInit(0x1CE654FCD4D50B22);
    nativePush(0);
    nativePush(24);
    if (!*nativeCall()) return;

    // CAM::GET_GAMEPLAY_CAM_ROT(2)
    nativeInit(0x837765A2533ECE65);
    nativePush(2);
    Vector3 camRot = *nativeCall();

    // CAM::GET_GAMEPLAY_CAM_COORD()
    nativeInit(0xFAAA931A783AEC66);
    Vector3 camCoord = *nativeCall();
    
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
    nativeInit(0x66C4C50F33CED8E8);
    nativePush(camCoord.x); nativePush(camCoord.y); nativePush(camCoord.z);
    nativePush(255); nativePush(0); nativePush(0);
    nativePush(30.0f); nativePush(15.0f);
    nativeCall();

    // GAMEPLAY::START_SHAPE_TEST_RAY(...)
    nativeInit(0x6A2924E9273DE2E6);
    nativePush(camCoord.x); nativePush(camCoord.y); nativePush(camCoord.z);
    nativePush(endCoords.x); nativePush(endCoords.y); nativePush(endCoords.z);
    nativePush(-1); nativePush(playerPed); nativePush(7);
    int raycast = *nativeCall();
    
    BOOL hit; Vector3 hitCoords; Vector3 surfaceNormal; Entity targetEntity;
    // GAMEPLAY::GET_SHAPE_TEST_RESULT(...)
    nativeInit(0x3D6CDA4C5305EDE2);
    nativePush(raycast); nativePush(&hit); nativePush(&hitCoords); nativePush(&surfaceNormal); nativePush(&targetEntity);
    nativeCall();

    // ENTITY::DOES_ENTITY_EXIST(targetEntity)
    nativeInit(0x5A504DE5EDE36555);
    nativePush(targetEntity);
    BOOL exists = *nativeCall();

    if (hit && exists)
    {
        // ENTITY::IS_ENTITY_A_PED(targetEntity)
        nativeInit(0x53351C66C3CD8132);
        nativePush(targetEntity);
        BOOL isPed = *nativeCall();

        if (isPed)
        {
            // PED::SET_PED_TO_RAGDOLL(...)
            nativeInit(0xAE99CC83A3C088E2);
            nativePush(targetEntity); nativePush(2000); nativePush(2000); nativePush(0);
            nativePush(true); nativePush(true); nativePush(false);
            nativeCall();

            // ENTITY::APPLY_FORCE_TO_ENTITY(...)
            nativeInit(0xC5F6E3E66F1CEDE4);
            nativePush(targetEntity); nativePush(1);
            nativePush(forwardVec.x * 60.0f); nativePush(forwardVec.y * 60.0f); nativePush(forwardVec.z * 35.0f);
            nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0);
            nativePush(false); nativePush(true); nativePush(true); nativePush(true); nativePush(true);
            nativeCall();
        }
        // ENTITY::IS_ENTITY_A_VEHICLE(targetEntity)
        else if (nativeInit(0x1253ECE4E50DE2E6), nativePush(targetEntity), (BOOL)*nativeCall())
        {
            // ENTITY::APPLY_FORCE_TO_ENTITY(...)
            nativeInit(0xC5F6E3E66F1CEDE4);
            nativePush(targetEntity); nativePush(1);
            nativePush(forwardVec.x * 120.0f); nativePush(forwardVec.y * 120.0f); nativePush(forwardVec.z * 70.0f);
            nativePush(0.0f); nativePush(0.0f); nativePush(0.5f); nativePush(0);
            nativePush(false); nativePush(true); nativePush(true); nativePush(true); nativePush(true);
            nativeCall();
        }
        
        // FIRE::START_ENTITY_FIRE(targetEntity)
        nativeInit(0xF64E4D3E6C2E2EE6);
        nativePush(targetEntity);
        nativeCall();
    }
}

void UpdateSupermanPhysics()
{
    // PLAYER::PLAYER_PED_ID()
    nativeInit(0x43A66C31C68491C0);
    Ped playerPed = *nativeCall();

    // ENTITY::IS_ENTITY_DEAD(playerPed)
    nativeInit(0x2D5C3E2C22D3E3E6);
    nativePush(playerPed);
    if (*nativeCall()) return;

    // CONTROLS::IS_CONTROL_JUST_PRESSED(0, 22)
    nativeInit(0x5F6D43E3882DEE22);
    nativePush(0);
    nativePush(22);
    if (*nativeCall()) 
    {
        g_isFlying = !g_isFlying;
        if (!g_isFlying)
        {
            // AI::STOP_ANIM_TASK(...)
            nativeInit(0x9924E2E6C35EDE44);
            nativePush(playerPed); nativePush("anim@animations"); nativePush("flight_loop"); nativePush(3.0f);
            nativeCall();

            // ENTITY::SET_ENTITY_HAS_GRAVITY(playerPed, true)
            nativeInit(0x21F3E3E66F2CEDE2);
            nativePush(playerPed); nativePush(true);
            nativeCall();
        }
        else
        {
            // ENTITY::SET_ENTITY_HAS_GRAVITY(playerPed, false)
            nativeInit(0x21F3E3E66F2CEDE2);
            nativePush(playerPed); nativePush(false);
            nativeCall();

            // AI::TASK_PLAY_ANIM(...)
            nativeInit(0x5A24E2E6C35EDE11);
            nativePush(playerPed); nativePush("anim@animations"); nativePush("flight_loop");
            nativePush(8.0f); nativePush(-8.0f); nativePush(-1); nativePush(9); nativePush(0.0f);
            nativePush(false); nativePush(false); nativePush(false);
            nativeCall();
        }
    }

    if (!g_isFlying) {
        TriggerHeatVisionJulioNIB();
        return;
    }

    // CONTROLS::GET_CONTROL_NORMAL(...)
    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(32); float wInput = *nativeCall();
    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(33); float sInput = *nativeCall();
    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(34); float aInput = *nativeCall();
    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(35); float dInput = *nativeCall();

    float forwardInput = wInput - sInput; 
    float turnInput = aInput - dInput;    

    // CONTROLS::IS_CONTROL_PRESSED(0, 21)
    nativeInit(0x1CE654FCD4D50B22); nativePush(0); nativePush(21);
    bool isBoosting = *nativeCall();

    // CAM::GET_GAMEPLAY_CAM_ROT(2)
    nativeInit(0x837765A2533ECE65); nativePush(2);
    Vector3 camRot = *nativeCall();

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
    nativeInit(0x15A66C31C68491F2);
    g_currentSpeed += acceleration * (float)*nativeCall();
    if (g_currentSpeed < 0.0f) g_currentSpeed = 0.0f;

    // ENTITY::SET_ENTITY_VELOCITY(...)
    nativeInit(0x74F6E3E66F1CEDE2);
    nativePush(playerPed);
    nativePush(flightDirection.x * g_currentSpeed);
    nativePush(flightDirection.y * g_currentSpeed);
    nativePush(flightDirection.z * g_currentSpeed);
    nativeCall();

    if (g_currentSpeed >= SOUND_SPEED && isBoosting)
    {
        // ENTITY::GET_ENTITY_COORDS(...)
        nativeInit(0x3C5C3E2C22D3E3E2); nativePush(playerPed); nativePush(true);
        Vector3 coords = *nativeCall();

        // FIRE::ADD_EXPLOSION(...)
        nativeInit(0x4201E3E66F1CEDE2);
        nativePush(coords.x); nativePush(coords.y); nativePush(coords.z);
        nativePush(34); nativePush(1.0f); nativePush(true); nativePush(false);
        nativeCall();

        // GAMEPLAY::SHAKE_GAMEPLAY_CAM(...)
        nativeInit(0x1201E3E66F1CEDE5);
        nativePush("LARGE_EXPLOSION_SHAKE"); nativePush(1.2f);
        nativeCall();
    }

    float targetLean = -turnInput * 45.0f; 
    g_currentLean = g_currentLean + (targetLean - g_currentLean) * 0.1f;
    
    // ENTITY::SET_ENTITY_ROTATION(...)
    nativeInit(0x82F6E3E66F1CEDE2);
    nativePush(playerPed); nativePush(camRot.x); nativePush(0.0f); nativePush(camRot.z + g_currentLean);
    nativePush(2); nativePush(true);
    nativeCall();

    TriggerHeatVisionJulioNIB();

    float normalizedSpeed = (g_currentSpeed / SOUND_SPEED);
    if (normalizedSpeed < 0.0f) normalizedSpeed = 0.0f;

    // ENTITY::GET_ENTITY_COORDS(...)
    nativeInit(0x3C5C3E2C22D3E3E2); nativePush(playerPed); nativePush(true);
    Vector3 pCoords = *nativeCall();
    
    // GRAPHICS::DRAW_MARKER(...)
    nativeInit(0x3201E3E66F1CEDE2);
    nativePush(1); nativePush(pCoords.x); nativePush(pCoords.y); nativePush(pos.z - 1.0f);
    nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0.0f);
    nativePush(2.0f); nativePush(2.0f); nativePush(0.5f);
    nativePush(255); nativePush(0); nativePush(0); nativePush((int)(normalizedSpeed * 255));
    nativePush(false); nativePush(true); nativePush(2); nativePush(false);
    nativePush(nullptr); nativePush(nullptr); nativePush(false);
    nativeCall();
}
