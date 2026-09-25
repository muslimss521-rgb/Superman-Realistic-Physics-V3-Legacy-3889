#include "Combat.h"
#include "main.h"
#include "natives.h"
#include <cmath>

namespace Combat
{
    void SuperPunch()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        // This SDK does not contain GET_GAMEPLAY_CAM_FORWARD_VECTOR.
        // Calculate the camera forward direction from GET_GAMEPLAY_CAM_ROT.
        Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);

        const float degToRad = 0.017453292519943295769f;
        const float pitch = rot.x * degToRad;
        const float yaw   = rot.z * degToRad;

        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);

        Vector3 forward;
        forward.x = -sy * cp;
        forward.y =  cy * cp;
        forward.z =  sp;

        // Short forward burst for the Superman punch.
        ENTITY::SET_ENTITY_VELOCITY(
            ped,
            forward.x * 6.0f,
            forward.y * 6.0f,
            forward.z * 6.0f + 1.5f);
    }

    void Update()
    {
    }
}
