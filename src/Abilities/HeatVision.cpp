#include "HeatVision.h"
#include "main.h"

namespace HeatVision {
    static bool g_active=false;
    void Toggle(){ g_active=!g_active; }
    bool Active(){ return g_active; }
    void Update(){
        if(!g_active) return;
        // Gameplay-native implementation placeholder; visual ray can be added with a compatible particle asset.
    }
}
