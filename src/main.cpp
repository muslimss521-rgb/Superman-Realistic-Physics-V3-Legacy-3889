#define NOMINMAX
#include <windows.h>
#include <string>
#include <algorithm>
#include <cmath>

// Подключаем типы данных ScriptHookV
#include "ScriptHookV/types.h"
#include "ScriptHookV/nativeCaller.h"
#include "ScriptHookV/main.h"

// Локальные конфигурации физики и заголовки
#include "Physics.h"
#include "Superman.h"

bool g_isFlying = false;
float g_currentSpeed = 0.0f;
float g_currentLean = 0.0f;

void TriggerHeatVisionJulioNIB()
{
    nativeInit(0x43A66C31C68491C0); // PLAYER_PED_ID
    Ped playerPed = *reinterpret_cast<Ped*>(nativeCall());

    nativeInit(0x1CE654FCD4D50B22); // IS_CONTROL_PRESSED
    nativePush(0);
    nativePush(24);
    if (!*reinterpret_cast<BOOL*>(nativeCall())) 
    {
        return;
    }

    float camRotX, camRotY, camRotZ;
    float camCoordX, camCoordY, camCoordZ;

    nativeInit(0x837765A2533ECE65); // GET_GAMEPLAY_CAM_ROT
    nativePush(2);
    Vector3 camRot = *reinterpret_cast<Vector3*>(nativeCall());
    camRotX = camRot.x;
    camRotY = camRot.y;
    camRotZ = camRot.z;

    nativeInit(0xFAAA931A783AEC66); // GET_GAMEPLAY_CAM_COORD
    Vector3 camCoord = *reinterpret_cast<Vector3*>(nativeCall());
    camCoordX = camCoord.x;
    camCoordY = camCoord.y;
    camCoordZ = camCoord.z;
    
    float pitch = camRotX * 0.0174532925f;
    float yaw = camRotZ * 0.0174532925f;
    
    Vector3 forwardVec;
    forwardVec.x = -sin(yaw) * cos(pitch);
    forwardVec.y = cos(yaw) * cos(pitch);
    forwardVec.z = sin(pitch);
    
    Vector3 endCoords;
    endCoords.x = camCoordX + forwardVec.x * 100.0f;
    endCoords.y = camCoordY + forwardVec.y * 100.0f;
    endCoords.z = camCoordZ + forwardVec.z * 100.0f;

    nativeInit(0x66C4C50F33CED8E8); // DRAW_LIGHT_WITH_RANGE
    nativePush(camCoordX); nativePush(camCoordY); nativePush(camCoordZ);
    nativePush(255); nativePush(0); nativePush(0);
    nativePush(30.0f); nativePush(15.0f);
    nativeCall();

    nativeInit(0x6A2924E9273DE2E6); // START_SHAPE_TEST_RAY
    nativePush(camCoordX); nativePush(camCoordY); nativePush(camCoordZ);
    nativePush(endCoords.x); nativePush(endCoords.y); nativePush(endCoords.z);
    nativePush(-1); nativePush(playerPed); nativePush(7);
    int raycast = *reinterpret_cast<int*>(nativeCall());
    
    BOOL hit = FALSE; Vector3 hitCoords = {0}; Vector3 surfaceNormal = {0}; Entity targetEntity = 0;
    nativeInit(0x3D6CDA4C5305EDE2); // GET_SHAPE_TEST_RESULT
    nativePush(raycast); nativePush(&hit); nativePush(&hitCoords); nativePush(&surfaceNormal); nativePush(&targetEntity);
    nativeCall();

    nativeInit(0x5A504DE5EDE36555); // DOES_ENTITY_EXIST
    nativePush(targetEntity);
    BOOL exists = *reinterpret_cast<BOOL*>(nativeCall());

    if (hit && exists)
    {
        nativeInit(0x53351C66C3CD8132); // IS_ENTITY_A_PED
        nativePush(targetEntity);
        BOOL isPed = *reinterpret_cast<BOOL*>(nativeCall());

        if (isPed)
        {
            nativeInit(0xAE99CC83A3C088E2); // SET_PED_TO_RAGDOLL
            nativePush(targetEntity); nativePush(2000); nativePush(2000); nativePush(0);
            nativePush(true); nativePush(true); nativePush(false);
            nativeCall();

            nativeInit(0xC5F6E3E66F1CEDE4); // APPLY_FORCE_TO_ENTITY
            nativePush(targetEntity); nativePush(1);
            nativePush(forwardVec.x * 60.0f); nativePush(forwardVec.y * 60.0f); nativePush(forwardVec.z * 35.0f);
            nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0);
            nativePush(false); nativePush(true); nativePush(true); nativePush(true); nativePush(true);
            nativeCall();
        }
        else
        {
            nativeInit(0x1253ECE4E50DE2E6); // IS_ENTITY_A_VEHICLE
            nativePush(targetEntity);
            BOOL isVehicle = *reinterpret_cast<BOOL*>(nativeCall());
            
            if (isVehicle)
            {
                nativeInit(0xC5F6E3E66F1CEDE4); // APPLY_FORCE_TO_ENTITY
                nativePush(targetEntity); nativePush(1);
                nativePush(forwardVec.x * 120.0f); nativePush(forwardVec.y * 120.0f); nativePush(forwardVec.z * 70.0f);
                nativePush(0.0f); nativePush(0.0f); nativePush(0.5f); nativePush(0);
                nativePush(false); nativePush(true); nativePush(true); nativePush(true); nativePush(true);
                nativeCall();
            }
        }
        
        nativeInit(0xF64E4D3E6C2E2EE6); // START_ENTITY_FIRE
        nativePush(targetEntity);
        nativeCall();
    }
}

void UpdateSupermanPhysics()
{
    nativeInit(0x43A66C31C68491C0); // PLAYER_PED_ID
    Ped playerPed = *reinterpret_cast<Ped*>(nativeCall());

    nativeInit(0x2D5C3E2C22D3E3E6); // IS_ENTITY_DEAD
    nativePush(playerPed);
    if (*reinterpret_cast<BOOL*>(nativeCall())) 
    {
        return;
    }

    nativeInit(0x5F6D43E3882DEE22); // IS_CONTROL_JUST_PRESSED
    nativePush(0);
    nativePush(22);
    if (*reinterpret_cast<BOOL*>(nativeCall())) 
    {
        g_isFlying = !g_isFlying;
        if (!g_isFlying)
        {
            nativeInit(0x9924E2E6C35EDE44); // STOP_ANIM_TASK
            nativePush(playerPed); nativePush("anim@animations"); nativePush("flight_loop"); nativePush(3.0f);
            nativeCall();

            nativeInit(0x21F3E3E66F2CEDE2); // SET_ENTITY_HAS_GRAVITY
            nativePush(playerPed); nativePush(true);
            nativeCall();
        }
        else
        {
            nativeInit(0x21F3E3E66F2CEDE2); // SET_ENTITY_HAS_GRAVITY
            nativePush(playerPed); nativePush(false);
            nativeCall();

            nativeInit(0x5A24E2E6C35EDE11); // TASK_PLAY_ANIM
            nativePush(playerPed); nativePush("anim@animations"); nativePush("flight_loop");
            nativePush(8.0f); nativePush(-8.0f); nativePush(-1); nativePush(9); nativePush(0.0f);
            nativePush(false); nativePush(false); nativePush(false);
            nativeCall();
        }
    }

    if (!g_isFlying) 
    {
        TriggerHeatVisionJulioNIB();
        return;
    }

    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(32); float wInput = *reinterpret_cast<float*>(nativeCall());
    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(33); float sInput = *reinterpret_cast<float*>(nativeCall());
    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(34); float aInput = *reinterpret_cast<float*>(nativeCall());
    nativeInit(0x32A66C31C68491A1); nativePush(0); nativePush(35); float dInput = *reinterpret_cast<float*>(nativeCall());

    float forwardInput = wInput - sInput; 
    float turnInput = aInput - dInput;    

    nativeInit(0x1CE654FCD4D50B22); nativePush(0); nativePush(21);
    bool isBoosting = *reinterpret_cast<BOOL*>(nativeCall());

    nativeInit(0x837765A2533ECE65); nativePush(2);
    Vector3 camRot = *reinterpret_cast<Vector3*>(nativeCall());

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
    
    nativeInit(0x15A66C31C68491F2); // GET_FRAME_TIME
    g_currentSpeed += acceleration * *reinterpret_cast<float*>(nativeCall());
    if (g_currentSpeed < 0.0f) 
    {
        g_currentSpeed = 0.0f;
    }

    nativeInit(0x74F6E3E66F1CEDE2); // SET_ENTITY_VELOCITY
    nativePush(playerPed);
    nativePush(flightDirection.x * g_currentSpeed);
    nativePush(flightDirection.y * g_currentSpeed);
    nativePush(flightDirection.z * g_currentSpeed);
    nativeCall();

    if (g_currentSpeed >= SOUND_SPEED && isBoosting)
    {
        nativeInit(0x3C5C3E2C22D3E3E2); nativePush(playerPed); nativePush(true); // GET_ENTITY_COORDS
        Vector3 coords = *reinterpret_cast<Vector3*>(nativeCall());

        nativeInit(0x4201E3E66F1CEDE2); // ADD_EXPLOSION
        nativePush(coords.x); nativePush(coords.y); nativePush(coords.z);
        nativePush(34); nativePush(1.0f); nativePush(true); nativePush(false);
        nativeCall();

        nativeInit(0x1201E3E66F1CEDE5); // SHAKE_GAMEPLAY_CAM
        nativePush("LARGE_EXPLOSION_SHAKE"); nativePush(1.2f);
        nativeCall();
    }

    float targetLean = -turnInput * 45.0f; 
    g_currentLean = g_currentLean + (targetLean - g_currentLean) * 0.1f;
    
    nativeInit(0x82F6E3E66F1CEDE2); // SET_ENTITY_ROTATION
    nativePush(playerPed); nativePush(camRot.x); nativePush(0.0f); nativePush(camRot.z + g_currentLean);
    nativePush(2); nativePush(true);
    nativeCall();

    TriggerHeatVisionJulioNIB();

    float normalizedSpeed = (g_currentSpeed / SOUND_SPEED);
    if (normalizedSpeed < 0.0f) 
    {
        normalizedSpeed = 0.0f;
    }

    nativeInit(0x3C5C3E2C22D3E3E2); nativePush(playerPed); nativePush(true);
    Vector3 pCoords = *reinterpret_cast<Vector3*>(nativeCall());
    
    nativeInit(0x3201E3E66F1CEDE2); // DRAW_MARKER
    nativePush(1); nativePush(pCoords.x); nativePush(pCoords.y); nativePush(pCoords.z - 1.0f);
    nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0.0f); nativePush(0.0f);
    nativePush(2.0f); nativePush(2.0f); nativePush(0.5f);
    nativePush(255); nativePush(0); nativePush(0); nativePush((int)(normalizedSpeed * 255));
    nativePush(false); nativePush(true); nativePush(2); nativePush(false);
    
    nativePush(0); nativePush(0); nativePush(false);
    nativeCall();
}

// ГЛАВНЫЙ ИГРОВОЙ ПОТОК — Скрепляет функции и запускает тики в GTA V каждый кадр [2]
void ScriptMain()
{
    srand(GetTickCount());
    while (true)
    {
        UpdateSupermanPhysics(); // Запуск тиков способностей и физики [2]
        scriptWait(0);          // Фиксация кадра ScriptHookV, чтобы игра не крашилась
    }
}
