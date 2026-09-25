# Superman — Final Architecture / GTA V Legacy 3889

Standalone ScriptHookV ASI project. No NIBSHDotNet or NIBSHDotNet_loader.

Implemented base:
- master toggle
- flight / hover / boost
- vertical and camera-relative movement
- emergency disable
- super-speed toggle
- target-lock module
- heat-vision module
- super-punch module
- modular structure ready for animation, VFX, sound, PED and additional powers

Controls:
F3 Superman ON/OFF
F5 Flight
W/S/A/D movement
Space/Ctrl vertical
Shift Boost
F6 Super Punch
F7 Target Lock
F9 Heat Vision
F10 Super Speed
F8 Emergency OFF

Build with Visual Studio MSBuild x64 Release.


## Анимации полёта

В проект добавлен `src/Abilities/FlightAnimation.cpp/.h`.

При включённом полёте используются встроенные GTA V анимации свободного падения:
- `free_idle` — зависание;
- `free_forward` — полёт вперёд;
- `free_backward` — движение назад;
- `free_left` / `free_right` — боковой полёт.

При Shift (Boost) скорость проигрывания полётной анимации повышается. Внешние `.ycd` для этого варианта не требуются.

Это именно GTA V-совместимый слой анимаций. Unreal `.uasset` из ManOfSteel напрямую в ASI не переносится.
