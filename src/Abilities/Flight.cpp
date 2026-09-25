#include "Flight.h"
#include "../Input.h"
#include "main.h"
#include <cmath>

namespace Flight {
    static bool g_flying = false;
    static bool g_boost = false;

    void Enable() { g_flying = true; }
    void Disable() {
        g_flying = false;
        g_boost = false;
        Ped ped = PLAYER::PLAYER_PED_ID();
        if (ENTITY::DOES_ENTITY_EXIST(ped)) {
            ENTITY::SET_ENTITY_HAS_GRAVITY(ped, true);
            ENTITY::SET_ENTITY_VELOCITY(ped, 0.0f, 0.0f, 0.0f);
        }
    }
    void SetBoost(bool enabled) { g_boost = enabled; }
    bool IsEnabled() { return g_flying; }

    void Update() {
        if (!g_flying) return;
        Ped ped = PLAYER::PLAYER_PED_ID();
        if (!ENTITY::DOES_ENTITY_EXIST(ped)) return;

        Vector3 f = CAM::GET_GAMEPLAY_CAM_FORWARD_VECTOR();
        Vector3 r = CAM::GET_GAMEPLAY_CAM_RIGHT_VECTOR();
        float speed = g_boost ? 3.5f : 1.0f;
        float x=0,y=0,z=0;

        if (Input::Forward()) { x += f.x*speed; y += f.y*speed; z += f.z*speed; }
        if (Input::Back())    { x -= f.x*speed; y -= f.y*speed; z -= f.z*speed; }
        if (Input::Left())    { x -= r.x*speed; y -= r.y*speed; }
        if (Input::Right())   { x += r.x*speed; y += r.y*speed; }
        if (Input::Up()) z += speed;
        if (Input::Down()) z -= speed;

        ENTITY::SET_ENTITY_HAS_GRAVITY(ped, false);
        ENTITY::SET_ENTITY_VELOCITY(ped, x, y, z);
    }
}
