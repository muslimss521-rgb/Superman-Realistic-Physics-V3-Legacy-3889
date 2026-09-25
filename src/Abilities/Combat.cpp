#include "Combat.h"
#include "main.h"
#include "natives.h"
#include <cmath>

namespace Combat
{
    static bool FindTarget(Ped player, Ped* outTarget)
    {
        if (!outTarget)
            return false;

        *outTarget = 0;

        Vector3 pos = ENTITY::GET_ENTITY_COORDS(player, TRUE);
        Ped target = 0;

        if (!PED::GET_CLOSEST_PED(
                pos.x, pos.y, pos.z,
                5.0f,
                TRUE, TRUE,
                &target,
                FALSE, FALSE,
                4))
        {
            return false;
        }

        if (target == 0 || target == player)
            return false;

        if (!ENTITY::DOES_ENTITY_EXIST(target))
            return false;

        *outTarget = target;
        return true;
    }

    void SuperPunch()
    {
        Ped player = PLAYER::PLAYER_PED_ID();
        if (!ENTITY::DOES_ENTITY_EXIST(player))
            return;

        Vector3 forward = ENTITY::GET_ENTITY_FORWARD_VECTOR(player);

        // Lunge forward.
        ENTITY::SET_ENTITY_VELOCITY(
            player,
            forward.x * 7.0f,
            forward.y * 7.0f,
            1.8f);

        Ped target = 0;
        if (FindTarget(player, &target))
        {
            ENTITY::SET_ENTITY_VELOCITY(
                target,
                forward.x * 18.0f,
                forward.y * 18.0f,
                7.0f);

            ENTITY::SET_ENTITY_HEALTH(
                target,
                0);
        }
    }

    void Update()
    {
    }
}
