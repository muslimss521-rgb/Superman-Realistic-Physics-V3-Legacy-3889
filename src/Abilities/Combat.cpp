#include "Combat.h"
#include "main.h"

namespace Combat {
    void SuperPunch(){
        Ped p=PLAYER::PLAYER_PED_ID();
        if(!ENTITY::DOES_ENTITY_EXIST(p)) return;
        Vector3 f=CAM::GET_GAMEPLAY_CAM_FORWARD_VECTOR();
        ENTITY::SET_ENTITY_VELOCITY(p, f.x*6.0f, f.y*6.0f, 1.5f);
    }
    void Update(){}
}
