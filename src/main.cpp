#include <windows.h>
#include <cmath>
#include <algorithm>

#include "ScriptHookV/main.h"
#include "ScriptHookV/natives.h"
#include "Superman.h"

static HMODULE g_module = nullptr;
static SupermanController g_superman;

static bool g_menuOpen = false;
static int g_selected = 0;

static bool lastF3 = false;
static bool lastUp = false;
static bool lastDown = false;
static bool lastEnter = false;

static DWORD g_lastGameTime = 0;
static DWORD g_lastAbilityTime = 0;

static const int MENU_COUNT = 12;

static const char* MENU[MENU_COUNT] =
{
    "Superman",
    "Flight",
    "Boost",
    "Super Speed",
    "Super Jump",
    "Invincibility",
    "Heat Vision",
    "Freeze Breath",
    "Super Breath",
    "Ground Pound",
    "Sonic Boom",
    "Solar Flare"
};

static bool Pressed(int key, bool& last)
{
    bool now = (GetAsyncKeyState(key) & 0x8000) != 0;
    bool result = now && !last;
    last = now;
    return result;
}

static void Text(const char* text, float x, float y, float scale)
{
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f, scale);
    UI::SET_TEXT_COLOUR(255, 255, 255, 255);
    UI::SET_TEXT_PROPORTIONAL(true);
    UI::SET_TEXT_OUTLINE();
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING((char*)text);
    UI::_DRAW_TEXT(x, y);
}

static void DrawMenu()
{
    if (!g_menuOpen)
        return;

    const float height = 0.085f + MENU_COUNT * 0.043f;

    GRAPHICS::DRAW_RECT(
        0.18f, 0.32f + height * 0.5f,
        0.34f, height,
        0, 0, 0, 215
    );

    GRAPHICS::DRAW_RECT(
        0.18f, 0.135f,
        0.34f, 0.055f,
        25, 70, 160, 240
    );

    Text("SUPERMAN", 0.055f, 0.112f, 0.50f);

    for (int i = 0; i < MENU_COUNT; ++i)
    {
        float y = 0.165f + i * 0.043f;

        if (i == g_selected)
        {
            GRAPHICS::DRAW_RECT(
                0.18f, y + 0.011f,
                0.31f, 0.036f,
                50, 110, 210, 220
            );
        }

        Text(MENU[i], 0.055f, y, 0.31f);
    }

    Text("F3 CLOSE", 0.055f, 0.165f + MENU_COUNT * 0.043f + 0.015f, 0.24f);
    Text("UP/DOWN SELECT", 0.055f, 0.165f + MENU_COUNT * 0.043f + 0.042f, 0.24f);
    Text("ENTER ACTIVATE", 0.055f, 0.165f + MENU_COUNT * 0.043f + 0.069f, 0.24f);
}

static void SupermanOn(Ped ped)
{
    g_superman.enabled = true;
    g_superman.abilities.state.godMode = true;

    ENTITY::SET_ENTITY_INVINCIBLE(ped, true);
    PED::SET_PED_MOVE_RATE_OVERRIDE(ped, 1.0f);
}

static void SupermanOff(Ped ped)
{
    g_superman.enabled = false;
    g_superman.abilities.state = AbilityState();

    ENTITY::SET_ENTITY_INVINCIBLE(ped, false);
    ENTITY::SET_ENTITY_HAS_GRAVITY(ped, true);
    PED::SET_PED_MOVE_RATE_OVERRIDE(ped, 1.0f);
    ENTITY::SET_ENTITY_VELOCITY(ped, 0.0f, 0.0f, 0.0f);

    g_superman.velocity = Vec3();
}

static Vector3 CameraForward()
{
    Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);

    const float d = 0.017453292519943295f;
    const float pitch = rot.x * d;
    const float yaw = rot.z * d;

    const float cp = std::cos(pitch);

    Vector3 f;
    f.x = -std::sin(yaw) * cp;
    f.y =  std::cos(yaw) * cp;
    f.z =  std::sin(pitch);
    return f;
}

static void BoostEffect(Ped ped, bool active, float speed)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);
    Vector3 forward = ENTITY::GET_ENTITY_FORWARD_VECTOR(ped);

    if (active)
    {
        const float intensity =
            (std::min)(2.8f, 0.45f + speed / 90.0f);

        Vector3 rear;
        rear.x = pos.x - forward.x * 1.5f;
        rear.y = pos.y - forward.y * 1.5f;
        rear.z = pos.z + 0.15f;

        GRAPHICS::DRAW_LIGHT_WITH_RANGE(
            rear.x, rear.y, rear.z,
            90, 190, 255,
            7.0f + intensity * 4.0f,
            12.0f + intensity * 10.0f
        );

        // Repeated short flame/smoke bursts give the boost a visible trail.
        GRAPHICS::START_PARTICLE_FX_NON_LOOPED_AT_COORD(
            (char*)"ent_sht_flame",
            rear.x, rear.y, rear.z,
            0.0f, 0.0f, ENTITY::GET_ENTITY_HEADING(ped),
            0.35f + intensity * 0.12f,
            false, false, false
        );

        if (speed > 150.0f)
        {
            Vector3 trail = rear;
            trail.x -= forward.x * 2.5f;
            trail.y -= forward.y * 2.5f;
            trail.z -= forward.z * 2.5f;

            GRAPHICS::START_PARTICLE_FX_NON_LOOPED_AT_COORD(
                (char*)"exp_grd_bzgas_smoke",
                trail.x, trail.y, trail.z,
                0.0f, 0.0f, ENTITY::GET_ENTITY_HEADING(ped),
                0.55f,
                false, false, false
            );
        }
    }
}

static void HeatVision(Ped ped)
{
    Vector3 start = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        ped, 0.0f, 0.55f, 0.55f
    );

    Vector3 f = CameraForward();

    Vector3 end;
    end.x = start.x + f.x * 55.0f;
    end.y = start.y + f.y * 55.0f;
    end.z = start.z + f.z * 55.0f;

    // A series of lights creates a continuous laser-like beam.
    for (int i = 1; i <= 8; ++i)
    {
        float t = (float)i / 8.0f;
        float x = start.x + (end.x - start.x) * t;
        float y = start.y + (end.y - start.y) * t;
        float z = start.z + (end.z - start.z) * t;

        GRAPHICS::DRAW_LIGHT_WITH_RANGE(
            x, y, z,
            255, 45, 20,
            1.3f,
            18.0f
        );
    }

    GRAPHICS::START_PARTICLE_FX_NON_LOOPED_AT_COORD(
        (char*)"ent_sht_flame",
        start.x, start.y, start.z,
        0.0f, 0.0f, ENTITY::GET_ENTITY_HEADING(ped),
        0.18f,
        false, false, false
    );
}

static void FreezeBreath(Ped ped)
{
    Vector3 start = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        ped, 0.0f, 0.8f, 0.55f
    );

    Vector3 f = CameraForward();

    for (int i = 1; i <= 6; ++i)
    {
        float t = (float)i / 6.0f;
        float x = start.x + f.x * (18.0f * t);
        float y = start.y + f.y * (18.0f * t);
        float z = start.z + f.z * (18.0f * t);

        GRAPHICS::DRAW_LIGHT_WITH_RANGE(
            x, y, z,
            150, 220, 255,
            1.5f,
            7.0f
        );
    }
}

static void SuperBreath(Ped ped)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);
    Vector3 f = CameraForward();

    Ped target = 0;
    if (PED::GET_CLOSEST_PED(
        pos.x + f.x * 4.0f,
        pos.y + f.y * 4.0f,
        pos.z + f.z * 4.0f,
        6.0f,
        true, true, &target, false, false, -1))
    {
        if (ENTITY::DOES_ENTITY_EXIST(target) && target != ped)
        {
            ENTITY::APPLY_FORCE_TO_ENTITY(
                target,
                1,
                f.x * 45.0f,
                f.y * 45.0f,
                f.z * 18.0f,
                0.0f, 0.0f, 0.0f,
                0, false, true, true, false, true
            );
        }
    }
}

static void GroundPound(Ped ped)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);

    ENTITY::SET_ENTITY_VELOCITY(
        ped, 0.0f, 0.0f, -65.0f
    );

    GRAPHICS::DRAW_LIGHT_WITH_RANGE(
        pos.x, pos.y, pos.z,
        255, 255, 255,
        10.0f,
        35.0f
    );
}

static void SonicBoom(Ped ped)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);

    GRAPHICS::DRAW_LIGHT_WITH_RANGE(
        pos.x, pos.y, pos.z,
        210, 240, 255,
        18.0f,
        45.0f
    );

    ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(
        ped,
        1,
        0.0f, 0.0f, 8.0f,
        false, false, true, false
    );
}

static void SolarFlare(Ped ped)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);

    GRAPHICS::DRAW_LIGHT_WITH_RANGE(
        pos.x, pos.y, pos.z + 1.0f,
        255, 235, 120,
        25.0f,
        80.0f
    );

    // Push nearby peds away without using a destructive explosion.
    Ped target = 0;
    if (PED::GET_CLOSEST_PED(
        pos.x, pos.y, pos.z,
        12.0f,
        true, true, &target, false, false, -1))
    {
        if (ENTITY::DOES_ENTITY_EXIST(target) && target != ped)
        {
            Vector3 tp = ENTITY::GET_ENTITY_COORDS(target, true);
            float dx = tp.x - pos.x;
            float dy = tp.y - pos.y;
            float dz = tp.z - pos.z;
            float len = std::sqrt(dx * dx + dy * dy + dz * dz);

            if (len > 0.1f)
            {
                dx /= len;
                dy /= len;
                dz /= len;

                ENTITY::APPLY_FORCE_TO_ENTITY(
                    target, 1,
                    dx * 30.0f,
                    dy * 30.0f,
                    dz * 15.0f,
                    0.0f, 0.0f, 0.0f,
                    0, false, true, true, false, true
                );
            }
        }
    }
}

static void Activate(Ped ped)
{
    switch (g_selected)
    {
        case 0:
            if (g_superman.enabled)
                SupermanOff(ped);
            else
                SupermanOn(ped);
            break;

        case 1:
            if (!g_superman.enabled)
                SupermanOn(ped);
            g_superman.abilities.state.flight =
                !g_superman.abilities.state.flight;
            ENTITY::SET_ENTITY_HAS_GRAVITY(
                ped,
                !g_superman.abilities.state.flight
            );
            break;

        case 2:
            if (!g_superman.enabled)
                SupermanOn(ped);
            g_superman.abilities.state.boost =
                !g_superman.abilities.state.boost;
            break;

        case 3:
            if (!g_superman.enabled)
                SupermanOn(ped);
            g_superman.abilities.state.superSpeed =
                !g_superman.abilities.state.superSpeed;
            break;

        case 4:
            if (!g_superman.enabled)
                SupermanOn(ped);
            GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(PLAYER::PLAYER_ID());
            break;

        case 5:
            if (!g_superman.enabled)
                SupermanOn(ped);
            g_superman.abilities.state.godMode =
                !g_superman.abilities.state.godMode;
            ENTITY::SET_ENTITY_INVINCIBLE(
                ped,
                g_superman.abilities.state.godMode
            );
            break;

        case 6:
            if (!g_superman.enabled)
                SupermanOn(ped);
            g_superman.abilities.state.heatVision =
                !g_superman.abilities.state.heatVision;
            break;

        case 7:
            if (!g_superman.enabled)
                SupermanOn(ped);
            g_superman.abilities.state.freezeBreath =
                !g_superman.abilities.state.freezeBreath;
            break;

        case 8:
            if (!g_superman.enabled)
                SupermanOn(ped);
            g_superman.abilities.state.superBreath =
                !g_superman.abilities.state.superBreath;
            break;

        case 9:
            GroundPound(ped);
            break;

        case 10:
            SonicBoom(ped);
            break;

        case 11:
            SolarFlare(ped);
            break;

        default:
            break;
    }
}

static void UpdateMenu(Ped ped)
{
    if (Pressed(VK_F3, lastF3))
        g_menuOpen = !g_menuOpen;

    if (!g_menuOpen)
        return;

    bool up = Pressed(VK_UP, lastUp);
    bool down = Pressed(VK_DOWN, lastDown);
    bool enter = Pressed(VK_RETURN, lastEnter);

    if (up)
    {
        --g_selected;
        if (g_selected < 0)
            g_selected = MENU_COUNT - 1;
    }

    if (down)
    {
        ++g_selected;
        if (g_selected >= MENU_COUNT)
            g_selected = 0;
    }

    if (enter)
        Activate(ped);
}

static void UpdateFlight(Ped ped, float dt)
{
    if (!g_superman.enabled ||
        !g_superman.abilities.state.flight)
        return;

    Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);

    const float d = 0.017453292519943295f;
    const float pitch = camRot.x * d;
    const float yaw = camRot.z * d;
    const float cp = std::cos(pitch);
    const float sp = std::sin(pitch);
    const float sy = std::sin(yaw);
    const float cy = std::cos(yaw);

    Vector3 forward;
    forward.x = -sy * cp;
    forward.y = cy * cp;
    forward.z = sp;

    Vector3 right;
    right.x = cy;
    right.y = sy;
    right.z = 0.0f;

    float fwd = 0.0f;
    float side = 0.0f;
    float vertical = 0.0f;

    if (GetAsyncKeyState('W') & 0x8000) fwd += 1.0f;
    if (GetAsyncKeyState('S') & 0x8000) fwd -= 1.0f;
    if (GetAsyncKeyState('D') & 0x8000) side += 1.0f;
    if (GetAsyncKeyState('A') & 0x8000) side -= 1.0f;
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) vertical += 1.0f;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) vertical -= 1.0f;

    const bool boost =
        g_superman.abilities.state.boost ||
        ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);

    const float maxSpeed = boost
        ? g_superman.boostSpeed
        : g_superman.flightSpeed;

    float inputLength =
        std::sqrt(fwd * fwd + side * side + vertical * vertical);

    if (inputLength > 1.0f)
    {
        fwd /= inputLength;
        side /= inputLength;
        vertical /= inputLength;
        inputLength = 1.0f;
    }

    Vector3 target;
    target.x = forward.x * fwd * maxSpeed + right.x * side * maxSpeed;
    target.y = forward.y * fwd * maxSpeed + right.y * side * maxSpeed;
    target.z = forward.z * fwd * maxSpeed + vertical * maxSpeed;

    Vector3 current = ENTITY::GET_ENTITY_VELOCITY(ped);

    const float acceleration = boost
        ? g_superman.boostAcceleration
        : g_superman.acceleration;

    float response = (std::min)(
        1.0f,
        (std::max)(0.01f, acceleration * dt / 25.0f)
    );

    Vector3 next;
    next.x = current.x + (target.x - current.x) * response;
    next.y = current.y + (target.y - current.y) * response;
    next.z = current.z + (target.z - current.z) * response;

    if (inputLength < 0.001f)
    {
        const float drag = boost ? 0.985f : 0.955f;
        next.x *= drag;
        next.y *= drag;
        next.z *= drag;
    }

    const float speed = std::sqrt(
        next.x * next.x +
        next.y * next.y +
        next.z * next.z
    );

    // Soft aerodynamic drag at high speed.
    if (speed > maxSpeed)
    {
        const float scale = maxSpeed / speed;
        next.x *= scale;
        next.y *= scale;
        next.z *= scale;
    }

    g_superman.velocity = Vec3(next.x, next.y, next.z);

    ENTITY::SET_ENTITY_HAS_GRAVITY(ped, false);
    ENTITY::SET_ENTITY_VELOCITY(
        ped, next.x, next.y, next.z
    );

    if (inputLength > 0.001f)
    {
        ENTITY::SET_ENTITY_ROTATION(
            ped,
            camRot.x,
            0.0f,
            camRot.z,
            2,
            true
        );
    }

    BoostEffect(ped, boost, speed);
}

static void UpdateAbilities(Ped ped, float dt)
{
    if (!g_superman.enabled)
        return;

    if (g_superman.abilities.state.superSpeed)
        PED::SET_PED_MOVE_RATE_OVERRIDE(ped, 2.0f);
    else
        PED::SET_PED_MOVE_RATE_OVERRIDE(ped, 1.0f);

    if (g_superman.abilities.state.godMode)
        ENTITY::SET_ENTITY_INVINCIBLE(ped, true);

    UpdateFlight(ped, dt);

    if (g_superman.abilities.state.heatVision)
        HeatVision(ped);

    if (g_superman.abilities.state.freezeBreath)
        FreezeBreath(ped);

    if (g_superman.abilities.state.superBreath)
    {
        if (GetAsyncKeyState('E') & 0x8000)
            SuperBreath(ped);
    }

    if (GetAsyncKeyState('G') & 0x8000)
    {
        if (g_superman.abilities.state.flight)
            GroundPound(ped);
    }

    // Flight physics is applied directly to the GTA entity above.
    // Keep the controller velocity synchronized for kinetic-energy calculations.
}

static void MainUpdate()
{
    Ped ped = PLAYER::PLAYER_PED_ID();

    if (!ENTITY::DOES_ENTITY_EXIST(ped))
        return;

    DWORD now = (DWORD)GAMEPLAY::GET_GAME_TIMER();

    if (g_lastGameTime == 0)
        g_lastGameTime = now;

    float dt =
        (float)(now - g_lastGameTime) / 1000.0f;

    g_lastGameTime = now;

    if (dt < 0.001f)
        dt = 0.001f;

    if (dt > 0.05f)
        dt = 0.05f;

    UpdateMenu(ped);

    if (g_superman.abilities.state.godMode)
        ENTITY::SET_ENTITY_INVINCIBLE(ped, true);

    UpdateAbilities(ped, dt);

    // One-shot visual abilities are rate-limited.
    if (now - g_lastAbilityTime > 700)
    {
        if (GetAsyncKeyState('Q') & 0x8000)
        {
            SonicBoom(ped);
            g_lastAbilityTime = now;
        }

        if (GetAsyncKeyState('F') & 0x8000)
        {
            SolarFlare(ped);
            g_lastAbilityTime = now;
        }
    }

    DrawMenu();
}

void ScriptMain()
{
    g_superman.Reset();
    g_lastGameTime = (DWORD)GAMEPLAY::GET_GAME_TIMER();

    while (true)
    {
        MainUpdate();
        WAIT(0);
    }
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved)
{
    (void)lpReserved;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        g_module = hModule;
        scriptRegister(g_module, ScriptMain);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        scriptUnregister(g_module);
        g_module = nullptr;
    }

    return TRUE;
}
