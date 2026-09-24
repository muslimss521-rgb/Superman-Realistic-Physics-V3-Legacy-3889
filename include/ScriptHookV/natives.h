#pragma once

#include "types.h"
#include "nativeCaller.h"

namespace PLAYER
{
    static Ped PLAYER_PED_ID() { return invoke<Ped>(0x43A66C31C68491C0); }
    // ... и так далее все остальные нативы игры
}
