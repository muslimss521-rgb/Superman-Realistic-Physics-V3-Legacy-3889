#include "TargetLock.h"
#include "main.h"

namespace TargetLock {
    static bool g_active=false;
    static Ped g_target=0;
    void Toggle(){ g_active=!g_active; if(!g_active) g_target=0; }
    bool Active(){ return g_active; }
    void Update(){
        if(!g_active) return;
        Ped p=PLAYER::PLAYER_PED_ID();
        if(!ENTITY::DOES_ENTITY_EXIST(p)) return;
        // Keep this module conservative for SDK compatibility; target acquisition is added later.
        if(g_target && !ENTITY::DOES_ENTITY_EXIST(g_target)) g_target=0;
    }
}
