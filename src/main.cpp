#include <windows.h>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <cstdio>

#include "ScriptHookV/main.h"
#include "ScriptHookV/natives.h"
#include "Superman.h"

static HMODULE g_module = nullptr;
static SupermanController g_superman;

static inline float MinF(float a, float b) { return (a < b) ? a : b; }
static inline float MaxF(float a, float b) { return (a > b) ? a : b; }

static bool g_menuOpen = false;
static int g_selected = 0;

static bool lastMenu = false;
static bool lastF3 = false;
static bool lastF5 = false;
static bool lastBack = false;
static bool lastUp = false;
static bool lastDown = false;
static bool lastEnter = false;

static bool lastFlight = false;
static bool lastSuperSpeed = false;
static bool lastInvincible = false;
static bool lastSuperJump = false;
static bool lastGroundPound = false;
static bool lastSonicBoom = false;
static bool lastSolarFlare = false;
static bool lastGrab = false;
static bool lastThrow = false;
static bool lastBulletTime = false;
static bool lastXRay = false;

static DWORD g_lastGameTime = 0;
static DWORD g_lastAbilityTime = 0;
static DWORD g_freezeReleaseTime = 0;

static Ped g_grabbedPed = 0;
static Ped g_frozenPed = 0;
static bool g_flightAnimLoaded = false;

static const char* FLIGHT_DICT = "skydive@base";
static const char* FLIGHT_ANIM = "free_idle";

struct Controls
{
    int menu = VK_F3;
    int flight = 'V';
    int boost = VK_LSHIFT;
    int superSpeed = 'G';
    int superJump = VK_SPACE;
    int invincible = 'I';
    int heatVision = 'H';
    int freezeBreath = 'J';
    int superBreath = 'K';
    int groundPound = 'X';
    int grab = 'E';
    int throwTarget = 'Q';
    int sonicBoom = 'R';
    int solarFlare = 'T';
    int bulletTime = 'B';
    int xrayVision = 'Y';

    int forward = 'W';
    int backward = 'S';
    int left = 'A';
    int right = 'D';
    int up = VK_SPACE;
    int down = VK_CONTROL;
};

static Controls g_keys;

static int ParseKey(const char* value, int fallback)
{
    if (!value || !value[0])
        return fallback;

    char key[32] = {};
    std::size_t n = std::strlen(value);
    if (n >= sizeof(key))
        n = sizeof(key) - 1;

    for (std::size_t i = 0; i < n; ++i)
        key[i] = (char)std::toupper((unsigned char)value[i]);

    key[n] = '\0';

    if (std::strlen(key) == 1)
    {
        unsigned char c = (unsigned char)key[0];

        if ((c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'))
            return (int)c;
    }

    if (!std::strcmp(key, "SPACE")) return VK_SPACE;
    if (!std::strcmp(key, "LSHIFT")) return VK_LSHIFT;
    if (!std::strcmp(key, "RSHIFT")) return VK_RSHIFT;
    if (!std::strcmp(key, "SHIFT")) return VK_SHIFT;
    if (!std::strcmp(key, "LCTRL")) return VK_LCONTROL;
    if (!std::strcmp(key, "RCTRL")) return VK_RCONTROL;
    if (!std::strcmp(key, "CTRL")) return VK_CONTROL;
    if (!std::strcmp(key, "ALT")) return VK_MENU;
    if (!std::strcmp(key, "ENTER")) return VK_RETURN;
    if (!std::strcmp(key, "BACKSPACE")) return VK_BACK;
    if (!std::strcmp(key, "TAB")) return VK_TAB;
    if (!std::strcmp(key, "UP")) return VK_UP;
    if (!std::strcmp(key, "DOWN")) return VK_DOWN;
    if (!std::strcmp(key, "LEFT")) return VK_LEFT;
    if (!std::strcmp(key, "RIGHT")) return VK_RIGHT;

    if (!std::strcmp(key, "F1")) return VK_F1;
    if (!std::strcmp(key, "F2")) return VK_F2;
    if (!std::strcmp(key, "F3")) return VK_F3;
    if (!std::strcmp(key, "F4")) return VK_F4;
    if (!std::strcmp(key, "F5")) return VK_F5;
    if (!std::strcmp(key, "F6")) return VK_F6;
    if (!std::strcmp(key, "F7")) return VK_F7;
    if (!std::strcmp(key, "F8")) return VK_F8;
    if (!std::strcmp(key, "F9")) return VK_F9;
    if (!std::strcmp(key, "F10")) return VK_F10;
    if (!std::strcmp(key, "F11")) return VK_F11;
    if (!std::strcmp(key, "F12")) return VK_F12;

    return fallback;
}

static void GetConfigPath(char* out, DWORD outSize)
{
    if (!out || outSize == 0)
        return;

    out[0] = '\0';

    char modulePath[MAX_PATH] = {};
    HMODULE module = g_module;

    if (module &&
        GetModuleFileNameA(module, modulePath, MAX_PATH) > 0)
    {
        char* slash = std::strrchr(modulePath, '\\');

        if (slash)
            *slash = '\0';

        std::snprintf(
            out,
            outSize,
            "%s\\config\\Superman.ini",
            modulePath
        );
        return;
    }

    std::strncpy(
        out,
        "config\\Superman.ini",
        outSize - 1
    );
    out[outSize - 1] = '\0';
}

static void ReadKey(const char* section, const char* name, const char* fallback, int& out)
{
    char value[32] = {};
    char configPath[MAX_PATH] = {};

    GetConfigPath(configPath, MAX_PATH);

    GetPrivateProfileStringA(
        section,
        name,
        fallback,
        value,
        sizeof(value),
        configPath
    );

    out = ParseKey(value, ParseKey(fallback, out));
}

static void LoadControls()
{
    ReadKey("Controls", "Toggle", "F3", g_keys.menu);
    ReadKey("Controls", "Flight", "V", g_keys.flight);
    ReadKey("Controls", "Boost", "LSHIFT", g_keys.boost);
    ReadKey("Controls", "SuperSpeed", "G", g_keys.superSpeed);
    ReadKey("Controls", "SuperJump", "SPACE", g_keys.superJump);
    ReadKey("Controls", "Invincibility", "I", g_keys.invincible);
    ReadKey("Controls", "HeatVision", "H", g_keys.heatVision);
    ReadKey("Controls", "FreezeBreath", "J", g_keys.freezeBreath);
    ReadKey("Controls", "SuperBreath", "K", g_keys.superBreath);
    ReadKey("Controls", "GroundPound", "X", g_keys.groundPound);
    ReadKey("Controls", "Grab", "E", g_keys.grab);
    ReadKey("Controls", "Throw", "Q", g_keys.throwTarget);
    ReadKey("Controls", "SonicBoom", "R", g_keys.sonicBoom);
    ReadKey("Controls", "SolarFlare", "T", g_keys.solarFlare);
    ReadKey("Controls", "BulletTime", "B", g_keys.bulletTime);
    ReadKey("Controls", "XRayVision", "Y", g_keys.xrayVision);

    ReadKey("Flight", "Forward", "W", g_keys.forward);
    ReadKey("Flight", "Backward", "S", g_keys.backward);
    ReadKey("Flight", "Left", "A", g_keys.left);
    ReadKey("Flight", "Right", "D", g_keys.right);
    ReadKey("Flight", "Up", "SPACE", g_keys.up);
    ReadKey("Flight", "Down", "CTRL", g_keys.down);
}

static bool Down(int key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

static bool Pressed(int key, bool& last)
{
    bool now = Down(key);
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

    static const char* MENU[] =
    {
        "Superman",
        "Flight [V]",
        "Boost [LShift]",
        "Super Speed [G]",
        "Super Jump [Space]",
        "Invincibility [I]",
        "Heat Vision [H]",
        "Freeze Breath [J]",
        "Super Breath [K]",
        "Ground Pound [X]",
        "Grab [E]",
        "Throw [Q]",
        "Sonic Boom [R]",
        "Solar Flare [T]",
        "Bullet Time [B]",
        "X-Ray Vision [Y]"
    };

    const int count = (int)(sizeof(MENU) / sizeof(MENU[0]));
    const float height = 0.085f + count * 0.043f;

    GRAPHICS::DRAW_RECT(
        0.18f, 0.32f + height * 0.5f,
        0.40f, height,
        0, 0, 0, 215
    );

    GRAPHICS::DRAW_RECT(
        0.18f, 0.135f,
        0.40f, 0.055f,
        25, 70, 160, 240
    );

    Text("SUPERMAN", 0.055f, 0.112f, 0.50f);

    for (int i = 0; i < count; ++i)
    {
        float y = 0.165f + i * 0.043f;

        if (i == g_selected)
        {
            GRAPHICS::DRAW_RECT(
                0.18f, y + 0.011f,
                0.37f, 0.036f,
                50, 110, 210, 220
            );
        }

        Text(MENU[i], 0.055f, y, 0.29f);
    }

    Text("F3 / F5 / BACKSPACE CLOSE", 0.055f, 0.165f + count * 0.043f + 0.015f, 0.23f);
    Text("UP/DOWN SELECT", 0.055f, 0.165f + count * 0.043f + 0.042f, 0.23f);
    Text("ENTER ACTIVATE", 0.055f, 0.165f + count * 0.043f + 0.069f, 0.23f);
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
    f.z = std::sin(pitch);
    return f;
}

static void StopFlightAnimation(Ped ped)
{
    if (!ENTITY::DOES_ENTITY_EXIST(ped))
        return;

    if (ENTITY::IS_ENTITY_PLAYING_ANIM(
        ped,
        (char*)FLIGHT_DICT,
        (char*)FLIGHT_ANIM,
        3))
    {
        AI::STOP_ANIM_TASK(
            ped,
            (char*)FLIGHT_DICT,
            (char*)FLIGHT_ANIM,
            2.0f
        );
    }

    g_flightAnimLoaded = false;
}

static void UpdateFlightAnimation(Ped ped)
{
    if (!ENTITY::DOES_ENTITY_EXIST(ped))
        return;

    STREAMING::REQUEST_ANIM_DICT((char*)FLIGHT_DICT);

    if (!STREAMING::HAS_ANIM_DICT_LOADED((char*)FLIGHT_DICT))
        return;

    g_flightAnimLoaded = true;

    if (!ENTITY::IS_ENTITY_PLAYING_ANIM(
        ped,
        (char*)FLIGHT_DICT,
        (char*)FLIGHT_ANIM,
        3))
    {
        // Vanilla GTA V free-flight pose. It replaces the normal running
        // animation while the entity is being driven through the air.
        AI::TASK_PLAY_ANIM(
            ped,
            (char*)FLIGHT_DICT,
            (char*)FLIGHT_ANIM,
            8.0f,
            -8.0f,
            -1,
            33,
            0.5f,
            false,
            false,
            false
        );
    }

    // Keep the body visually static instead of cycling through a run/fall
    // animation. The entity's position/velocity is still controlled by physics.
    ENTITY::SET_ENTITY_ANIM_CURRENT_TIME(
        ped,
        (char*)FLIGHT_DICT,
        (char*)FLIGHT_ANIM,
        0.50f
    );
}

static void BoostEffect(Ped ped, bool active, float speed)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);
    Vector3 forward = ENTITY::GET_ENTITY_FORWARD_VECTOR(ped);

    if (!active)
        return;

    const float intensity =
        MinF(3.0f, 0.45f + speed / 85.0f);

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

    if (speed > 180.0f)
        CAM::SET_GAMEPLAY_CAM_SHAKE_AMPLITUDE(
            MinF(0.35f, 0.04f + speed / 1000.0f)
        );
}

static Ped FindTargetInFront(Ped player, float distance, float radius)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(player, true);
    Vector3 f = CameraForward();

    Ped target = 0;

    if (PED::GET_CLOSEST_PED(
        pos.x + f.x * distance,
        pos.y + f.y * distance,
        pos.z + f.z * distance,
        radius,
        true, true, &target, false, false, -1))
    {
        if (ENTITY::DOES_ENTITY_EXIST(target) && target != player)
            return target;
    }

    return 0;
}

static void HeatVision(Ped ped)
{
    const float maxDistance = 100.0f;
    const float pushForce = 50.0f;

    Vector3 start = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        ped, 0.0f, 0.55f, 0.55f);

    Vector3 f = CameraForward();

    Vector3 end;
    end.x = start.x + f.x * maxDistance;
    end.y = start.y + f.y * maxDistance;
    end.z = start.z + f.z * maxDistance;

    for (int i = 1; i <= 18; ++i)
    {
        float t = (float)i / 18.0f;

        GRAPHICS::DRAW_LIGHT_WITH_RANGE(
            start.x + (end.x - start.x) * t,
            start.y + (end.y - start.y) * t,
            start.z + (end.z - start.z) * t,
            255, 45, 20,
            1.5f,
            12.0f
        );
    }

    Ped target = FindTargetInFront(ped, 20.0f, 4.0f);

    if (target != 0)
    {
        ENTITY::APPLY_FORCE_TO_ENTITY(
            target,
            1,
            f.x * pushForce,
            f.y * pushForce,
            f.z * pushForce,
            0.0f, 0.0f, 0.0f,
            0, false, true, true, false, true
        );

        if (g_lastGameTime - g_lastAbilityTime > 90)
        {
            PED::APPLY_DAMAGE_TO_PED(target, 8, true);
            g_lastAbilityTime = g_lastGameTime;
        }
    }
}

static void FreezeBreath(Ped ped)
{
    const float freezeRange = 15.0f;
    const float coneAngle = 30.0f;

    Vector3 start = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
        ped, 0.0f, 0.8f, 0.55f);

    Vector3 f = CameraForward();

    for (int i = 1; i <= 12; ++i)
    {
        float t = (float)i / 12.0f;

        GRAPHICS::DRAW_LIGHT_WITH_RANGE(
            start.x + f.x * (freezeRange * t),
            start.y + f.y * (freezeRange * t),
            start.z + f.z * (freezeRange * t),
            150, 220, 255,
            1.8f,
            9.0f
        );
    }

    int handles[256] = {};
    int count = worldGetAllPeds(handles, 256);

    if (count < 0)
        count = 0;
    if (count > 256)
        count = 256;

    for (int i = 0; i < count; ++i)
    {
        Ped target = (Ped)handles[i];

        if (!ENTITY::DOES_ENTITY_EXIST(target) || target == ped)
            continue;

        Vector3 p = ENTITY::GET_ENTITY_COORDS(target, true);

        float dx = p.x - start.x;
        float dy = p.y - start.y;
        float dz = p.z - start.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist <= 0.1f || dist > freezeRange)
            continue;

        dx /= dist;
        dy /= dist;
        dz /= dist;

        float dot = f.x * dx + f.y * dy + f.z * dz;
        dot = MaxF(-1.0f, MinF(1.0f, dot));

        float angle =
            std::acos(dot) * 57.2957795f;

        if (angle > coneAngle)
            continue;

        ENTITY::FREEZE_ENTITY_POSITION(target, true);
        PED::SET_PED_CAN_RAGDOLL(target, false);

        if (g_frozenPed == 0)
            g_frozenPed = target;

        g_freezeReleaseTime = g_lastGameTime + 1000;
    }
}

static void ReleaseFrozenTarget()
{
    if (g_frozenPed != 0 &&
        ENTITY::DOES_ENTITY_EXIST(g_frozenPed))
    {
        ENTITY::FREEZE_ENTITY_POSITION(
            g_frozenPed,
            false
        );

        PED::SET_PED_CAN_RAGDOLL(
            g_frozenPed,
            true
        );
    }

    g_frozenPed = 0;
    g_freezeReleaseTime = 0;
}

static void SuperBreath(Ped ped)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);
    Vector3 f = CameraForward();

    Ped target = FindTargetInFront(ped, 4.0f, 6.0f);

    if (target != 0)
    {
        ENTITY::APPLY_FORCE_TO_ENTITY(
            target,
            1,
            f.x * 55.0f,
            f.y * 55.0f,
            f.z * 22.0f,
            0.0f, 0.0f, 0.0f,
            0, false, true, true, false, true
        );
    }

    GRAPHICS::DRAW_LIGHT_WITH_RANGE(
        pos.x + f.x * 2.0f,
        pos.y + f.y * 2.0f,
        pos.z + f.z * 2.0f,
        170, 220, 255,
        3.0f,
        16.0f
    );
}


static void UpdateXRay(Ped player, bool active)
{
    int handles[512] = {};
    int count = worldGetAllPeds(handles, 512);

    if (count < 0)
        count = 0;
    if (count > 512)
        count = 512;

    Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(player, true);

    for (int i = 0; i < count; ++i)
    {
        Ped target = (Ped)handles[i];

        if (!ENTITY::DOES_ENTITY_EXIST(target) ||
            target == player)
            continue;

        Vector3 p = ENTITY::GET_ENTITY_COORDS(target, true);
        float dx = p.x - playerPos.x;
        float dy = p.y - playerPos.y;
        float dz = p.z - playerPos.z;
        float dist2 = dx * dx + dy * dy + dz * dz;

        if (dist2 > 80.0f * 80.0f)
            continue;

        if (active)
        {
            ENTITY::SET_ENTITY_ALWAYS_PRERENDER(target, true);
            ENTITY::SET_ENTITY_ALPHA(target, 150, false);

            GRAPHICS::DRAW_LINE(
                playerPos.x, playerPos.y, playerPos.z + 0.6f,
                p.x, p.y, p.z + 0.6f,
                80, 210, 255, 150
            );

            GRAPHICS::DRAW_MARKER(
                28,
                p.x, p.y, p.z + 1.0f,
                0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f,
                0.22f, 0.22f, 0.22f,
                80, 210, 255, 180,
                false, false, 2, false,
                nullptr, nullptr, false
            );
        }
        else
        {
            ENTITY::RESET_ENTITY_ALPHA(target);
            ENTITY::SET_ENTITY_ALWAYS_PRERENDER(target, false);
        }
    }
}

static void GroundPound(Ped ped)
{
    ENTITY::SET_ENTITY_VELOCITY(
        ped, 0.0f, 0.0f, -85.0f
    );

    g_superman.velocity.z = -85.0f;
}

static void GroundPoundImpact(Ped ped)
{
    Vector3 pos = ENTITY::GET_ENTITY_COORDS(ped, true);
    Vector3 vel = ENTITY::GET_ENTITY_VELOCITY(ped);

    float impactSpeed = std::fabs(vel.z);
    if (impactSpeed < 15.0f)
        impactSpeed = 15.0f;

    const float mass = 95.0f;
    const float energy = 0.5f * mass * impactSpeed * impactSpeed;

    GRAPHICS::DRAW_LIGHT_WITH_RANGE(
        pos.x, pos.y, pos.z,
        255, 255, 255,
        MinF(20.0f, 8.0f + impactSpeed * 0.15f),
        MinF(65.0f, 25.0f + impactSpeed * 0.45f)
    );

    int handles[256] = {};
    int count = worldGetAllPeds(handles, 256);

    if (count < 0)
        count = 0;
    if (count > 256)
        count = 256;

    const float radius = 20.0f;

    for (int i = 0; i < count; ++i)
    {
        Ped target = (Ped)handles[i];

        if (!ENTITY::DOES_ENTITY_EXIST(target) || target == ped)
            continue;

        Vector3 tp = ENTITY::GET_ENTITY_COORDS(target, true);

        float dx = tp.x - pos.x;
        float dy = tp.y - pos.y;
        float dz = tp.z - pos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist <= 0.1f || dist > radius)
            continue;

        float falloff = 1.0f - dist / radius;
        float impulse = MinF(
            180.0f,
            20.0f + energy * 0.035f * falloff
        );

        dx /= dist;
        dy /= dist;

        ENTITY::APPLY_FORCE_TO_ENTITY(
            target, 1,
            dx * impulse,
            dy * impulse,
            (12.0f + impactSpeed * 0.30f) * falloff,
            0.0f, 0.0f, 0.0f,
            0, false, true, true, false, true
        );
    }
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

    Ped target = 0;

    if (PED::GET_CLOSEST_PED(
        pos.x, pos.y, pos.z,
        18.0f,
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
                    dx * 75.0f,
                    dy * 75.0f,
                    dz * 35.0f,
                    0.0f, 0.0f, 0.0f,
                    0, false, true, true, false, true
                );
            }
        }
    }
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
                    dx * 45.0f,
                    dy * 45.0f,
                    dz * 25.0f,
                    0.0f, 0.0f, 0.0f,
                    0, false, true, true, false, true
                );
            }
        }
    }
}

static void Grab(Ped ped)
{
    if (g_grabbedPed != 0)
        return;

    Ped target = FindTargetInFront(ped, 2.5f, 2.5f);

    if (target == 0)
        return;

    int handBone = PED::GET_PED_BONE_INDEX(
        ped,
        57005
    );

    ENTITY::SET_ENTITY_HAS_GRAVITY(
        target,
        false
    );

    PED::SET_PED_CAN_RAGDOLL(
        target,
        false
    );

    ENTITY::ATTACH_ENTITY_TO_ENTITY(
        target,
        ped,
        handBone,
        0.35f, 0.65f, -0.15f,
        0.0f, 0.0f, 90.0f,
        false,
        false,
        false,
        true,
        0,
        true
    );

    g_grabbedPed = target;
    g_superman.abilities.state.grabbing = true;
}

static void ThrowGrabbed(Ped ped)
{
    if (g_grabbedPed == 0)
        return;

    Ped target = g_grabbedPed;

    if (ENTITY::DOES_ENTITY_EXIST(target))
    {
        ENTITY::DETACH_ENTITY(
            target,
            true,
            true
        );

        ENTITY::SET_ENTITY_HAS_GRAVITY(
            target,
            true
        );

        PED::SET_PED_CAN_RAGDOLL(
            target,
            true
        );

        Vector3 f = CameraForward();

        ENTITY::APPLY_FORCE_TO_ENTITY(
            target,
            1,
            f.x * 180.0f,
            f.y * 180.0f,
            f.z * 80.0f + 20.0f,
            0.0f, 0.0f, 0.0f,
            0, false, true, true, false, true
        );
    }

    g_grabbedPed = 0;
    g_superman.abilities.state.grabbing = false;
}

static void SupermanOn(Ped ped)
{
    g_superman.enabled = true;

    ENTITY::SET_ENTITY_INVINCIBLE(
        ped,
        g_superman.abilities.state.godMode
    );
}

static void SupermanOff(Ped ped)
{
    if (g_grabbedPed != 0)
        ThrowGrabbed(ped);

    ReleaseFrozenTarget();
    StopFlightAnimation(ped);

    g_superman.enabled = false;
    g_superman.abilities.state = AbilityState();
    UpdateXRay(ped, false);

    ENTITY::SET_ENTITY_INVINCIBLE(ped, false);
    ENTITY::SET_ENTITY_HAS_GRAVITY(ped, true);
    PED::SET_PED_CAN_RAGDOLL(ped, true);
    PED::SET_PED_MOVE_RATE_OVERRIDE(ped, 1.0f);
    ENTITY::SET_ENTITY_VELOCITY(ped, 0.0f, 0.0f, 0.0f);

    CAM::SET_GAMEPLAY_CAM_SHAKE_AMPLITUDE(0.0f);

    g_superman.velocity = Vec3();
}

static void ActivateMenu(Ped ped)
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
            g_superman.abilities.state.flight =
                !g_superman.abilities.state.flight;
            break;

        case 2:
            g_superman.abilities.state.boost =
                !g_superman.abilities.state.boost;
            break;

        case 3:
            g_superman.abilities.state.superSpeed =
                !g_superman.abilities.state.superSpeed;
            break;

        case 4:
            g_superman.abilities.state.flight = false;
            GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(
                PLAYER::PLAYER_ID()
            );
            break;

        case 5:
            g_superman.abilities.state.godMode =
                !g_superman.abilities.state.godMode;
            ENTITY::SET_ENTITY_INVINCIBLE(
                ped,
                g_superman.abilities.state.godMode
            );
            break;

        case 6:
            g_superman.abilities.state.heatVision =
                !g_superman.abilities.state.heatVision;
            break;

        case 7:
            g_superman.abilities.state.freezeBreath =
                !g_superman.abilities.state.freezeBreath;
            break;

        case 8:
            g_superman.abilities.state.superBreath =
                !g_superman.abilities.state.superBreath;
            break;

        case 9:
            GroundPound(ped);
            break;

        case 10:
            Grab(ped);
            break;

        case 11:
            ThrowGrabbed(ped);
            break;

        case 12:
            SonicBoom(ped);
            break;

        case 13:
            SolarFlare(ped);
            break;

        case 14:
            g_superman.abilities.state.bulletTime =
                !g_superman.abilities.state.bulletTime;
            break;

        case 15:
            g_superman.abilities.state.xrayVision =
                !g_superman.abilities.state.xrayVision;
            break;

        default:
            break;
    }
}

static void UpdateMenu(Ped ped)
{
    // F3 remains a guaranteed emergency/default menu key even if
    // Superman.ini is missing or the configured key was changed.
    bool configuredMenuPressed = Pressed(g_keys.menu, lastMenu);
    bool f3Pressed = Pressed(VK_F3, lastF3);
    bool f5Pressed = Pressed(VK_F5, lastF5);
    bool backPressed = Pressed(VK_BACK, lastBack);

    if (configuredMenuPressed || f3Pressed || f5Pressed)
        g_menuOpen = !g_menuOpen;

    if (g_menuOpen && backPressed)
        g_menuOpen = false;

    if (!g_menuOpen)
        return;

    if (Pressed(VK_UP, lastUp))
    {
        --g_selected;
        if (g_selected < 0)
            g_selected = 15;
    }

    if (Pressed(VK_DOWN, lastDown))
    {
        ++g_selected;
        if (g_selected > 15)
            g_selected = 0;
    }

    if (Pressed(VK_RETURN, lastEnter))
        ActivateMenu(ped);
}

static float g_flightLean = 0.0f;
static float g_flightPitch = 0.0f;

static void ApplyFlightForce(Ped ped, const Vector3& force)
{
    // ForceType 1 = continuous force. We deliberately use F = m * a here,
    // instead of teleporting the PED or directly overwriting its velocity.
    ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(
        ped,
        1,
        force.x,
        force.y,
        force.z,
        false,
        true,
        true,
        false
    );
}

static void UpdateFlight(Ped ped, float dt)
{
    if (!g_superman.enabled ||
        !g_superman.abilities.state.flight)
    {
        StopFlightAnimation(ped);
        g_flightLean = 0.0f;
        g_flightPitch = 0.0f;
        return;
    }

    if (!ENTITY::DOES_ENTITY_EXIST(ped))
        return;

    // ------------------------------------------------------------
    // SUPERMAN FLIGHT MODEL
    // ------------------------------------------------------------
    // The character remains subject to GTA gravity. Every frame we apply
    // physical forces instead of moving him with SET_ENTITY_COORDS or
    // directly forcing a target position. This gives real inertia:
    //       F = m * a
    //       p = m * v
    //       E = 1/2 * m * v^2
    // and makes acceleration/deceleration depend on the current velocity.
    //
    // The supplied Unity reference is kept as the design baseline:
    // flyForce=30, maxSpeed=50, boost=2.5, airDrag=1, stopDrag=5.
    // The GTA implementation adds Newton-style mass, lift and quadratic
    // air resistance on top of that baseline.
    // ------------------------------------------------------------

    PED::SET_PED_CAN_RAGDOLL(ped, false);
    ENTITY::SET_ENTITY_HAS_GRAVITY(ped, true);

    UpdateFlightAnimation(ped);

    float frame = dt;
    if (frame < 0.008f) frame = 0.008f;
    if (frame > 0.050f) frame = 0.050f;

    const float mass =
        (g_superman.mass > 1.0f)
        ? g_superman.mass
        : 95.0f;

    Vector3 forward = CameraForward();

    // Horizontal right vector. We keep vertical steering separate so that
    // A/D does not unexpectedly push Superman up or down.
    Vector3 right;
    right.x = forward.y;
    right.y = -forward.x;
    right.z = 0.0f;

    float rightLen =
        std::sqrt(right.x * right.x + right.y * right.y);

    if (rightLen > 0.001f)
    {
        right.x /= rightLen;
        right.y /= rightLen;
    }

    float fwdInput = 0.0f;
    float sideInput = 0.0f;
    float verticalInput = 0.0f;

    if (Down(g_keys.forward))  fwdInput += 1.0f;
    if (Down(g_keys.backward)) fwdInput -= 1.0f;
    if (Down(g_keys.right))    sideInput += 1.0f;
    if (Down(g_keys.left))     sideInput -= 1.0f;
    if (Down(g_keys.up))       verticalInput += 1.0f;
    if (Down(g_keys.down))     verticalInput -= 1.0f;

    const bool boosting =
        Down(g_keys.boost) ||
        g_superman.abilities.state.boost;

    const float baseMaxSpeed =
        (g_superman.flightSpeed > 1.0f)
        ? g_superman.flightSpeed
        : 50.0f;

    const float maxSpeed =
        boosting
        ? baseMaxSpeed * g_superman.boostMultiplier
        : baseMaxSpeed;

    const float thrustAcceleration =
        boosting
        ? ((g_superman.boostAcceleration > 1.0f)
            ? g_superman.boostAcceleration
            : 75.0f)
        : ((g_superman.flightForce > 1.0f)
            ? g_superman.flightForce
            : 30.0f);

    Vector3 velocity = ENTITY::GET_ENTITY_VELOCITY(ped);

    const float speed =
        std::sqrt(
            velocity.x * velocity.x +
            velocity.y * velocity.y +
            velocity.z * velocity.z
        );

    // ------------------------------------------------------------
    // 1. NEWTON GRAVITY + HOVER STABILIZER
    // ------------------------------------------------------------
    // GTA gravity pulls down continuously. When Superman is flying but the
    // player gives no vertical command, he must produce approximately mg of
    // lift to hover. A small velocity damping term prevents oscillation.
    // This is why he can remain in the air instead of slowly falling.
    const float gravity = Physics::EarthGravity;
    const float gravityForce = mass * gravity;

    float liftRatio = 1.0f;

    // At forward flight speeds, the body can use aerodynamic lift. We still
    // keep a small reserve of active lift so he does not suddenly drop when
    // speed changes. At zero speed the active lift remains exactly enough to
    // counter gravity, producing a stable cinematic hover.
    if (fwdInput > 0.0f && speed > 8.0f)
    {
        float speedLift = speed / 55.0f;
        if (speedLift > 1.0f) speedLift = 1.0f;
        liftRatio = 0.90f + 0.10f * speedLift;
    }

    float liftForce = gravityForce * liftRatio;

    // Vertical damping makes the hover behave like a controlled human-scale
    // flight system rather than a frozen object.
    const float verticalDamping = 2.4f;
    float hoverCorrection =
        -velocity.z * verticalDamping * mass;

    // The correction is deliberately limited so it cannot create a sudden
    // launch when entering flight mode.
    const float maxHoverCorrection = gravityForce * 0.65f;
    if (hoverCorrection > maxHoverCorrection)
        hoverCorrection = maxHoverCorrection;
    if (hoverCorrection < -maxHoverCorrection)
        hoverCorrection = -maxHoverCorrection;

    // Vertical input changes the desired acceleration. This is added on top
    // of the gravity-cancelling hover force.
    const float verticalAcceleration =
        thrustAcceleration * 0.72f;

    Vector3 totalForce{};
    totalForce.x = 0.0f;
    totalForce.y = 0.0f;
    totalForce.z =
        liftForce + hoverCorrection +
        verticalInput * mass * verticalAcceleration;

    // ------------------------------------------------------------
    // 2. FORWARD / BACKWARD THRUST
    // ------------------------------------------------------------
    // W applies a force in the camera direction. S applies a softer reverse
    // force, preserving inertia instead of snapping the velocity backwards.
    if (fwdInput > 0.0f)
    {
        totalForce.x += forward.x * mass * thrustAcceleration * fwdInput;
        totalForce.y += forward.y * mass * thrustAcceleration * fwdInput;
        totalForce.z += forward.z * mass * thrustAcceleration * fwdInput;
    }
    else if (fwdInput < 0.0f)
    {
        const float reverseAcceleration =
            thrustAcceleration * 0.45f;

        totalForce.x += forward.x * mass * reverseAcceleration * fwdInput;
        totalForce.y += forward.y * mass * reverseAcceleration * fwdInput;
        totalForce.z += forward.z * mass * reverseAcceleration * fwdInput;
    }

    // ------------------------------------------------------------
    // 3. SIDE FORCE
    // ------------------------------------------------------------
    // A/D is lateral thrust, not an instantaneous position change.
    const float sideAcceleration =
        thrustAcceleration * 0.60f;

    totalForce.x += right.x * mass * sideAcceleration * sideInput;
    totalForce.y += right.y * mass * sideAcceleration * sideInput;

    // ------------------------------------------------------------
    // 4. QUADRATIC AIR RESISTANCE
    // ------------------------------------------------------------
    // F_drag = 1/2 * rho * Cd * A * v^2.
    // The coefficient is tuned for GTA's world scale rather than real SI
    // air density, so the result remains controllable at game speeds.
    float airDensity = 1.225f;
    float dragCoefficient =
        (std::fabs(fwdInput) > 0.001f ||
         std::fabs(sideInput) > 0.001f ||
         std::fabs(verticalInput) > 0.001f)
        ? 0.42f
        : 0.90f;

    float referenceArea = 0.85f;

    if (speed > 0.05f)
    {
        float dragMagnitude =
            0.5f *
            airDensity *
            dragCoefficient *
            referenceArea *
            speed * speed;

        // Scale the physical drag for GTA's velocity units.
        dragMagnitude *= 0.095f;

        totalForce.x -= velocity.x / speed * dragMagnitude;
        totalForce.y -= velocity.y / speed * dragMagnitude;
        totalForce.z -= velocity.z / speed * dragMagnitude;
    }

    // Additional controllable braking when the player completely releases
    // the controls. This keeps the cinematic flight from drifting forever.
    const bool noInput =
        std::fabs(fwdInput) < 0.001f &&
        std::fabs(sideInput) < 0.001f &&
        std::fabs(verticalInput) < 0.001f;

    if (noInput && speed > 0.05f)
    {
        const float brakingAcceleration =
            (g_superman.stopDrag > 0.0f)
            ? g_superman.stopDrag * 0.75f
            : 3.0f;

        totalForce.x -= velocity.x / speed * mass * brakingAcceleration;
        totalForce.y -= velocity.y / speed * mass * brakingAcceleration;
        totalForce.z -= velocity.z / speed * mass * brakingAcceleration;
    }

    // ------------------------------------------------------------
    // 5. APPLY F = m * a
    // ------------------------------------------------------------
    ApplyFlightForce(ped, totalForce);

    // Read the resulting velocity after the force application for visual
    // orientation and effects. We do not overwrite it with SET_ENTITY_VELOCITY.
    Vector3 actualVelocity = ENTITY::GET_ENTITY_VELOCITY(ped);

    float actualSpeed =
        std::sqrt(
            actualVelocity.x * actualVelocity.x +
            actualVelocity.y * actualVelocity.y +
            actualVelocity.z * actualVelocity.z
        );

    // ------------------------------------------------------------
    // 6. SPEED LIMIT WITHOUT DESTROYING INERTIA
    // ------------------------------------------------------------
    // We only trim velocity if it exceeds the configured maximum. Normal
    // acceleration/deceleration remains completely physics-driven.
    if (actualSpeed > maxSpeed && actualSpeed > 0.001f)
    {
        float excess = actualSpeed - maxSpeed;
        float correction =
            MinF(excess, maxSpeed * 0.20f * frame);

        const float trimScale = correction / actualSpeed;
        Vector3 trim{};
        trim.x = actualVelocity.x * trimScale;
        trim.y = actualVelocity.y * trimScale;
        trim.z = actualVelocity.z * trimScale;

        ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(
            ped,
            1,
            -trim.x * mass / frame,
            -trim.y * mass / frame,
            -trim.z * mass / frame,
            false,
            true,
            true,
            false
        );

        actualVelocity.x -= trim.x;
        actualVelocity.y -= trim.y;
        actualVelocity.z -= trim.z;
        actualSpeed -= correction;
    }

    g_superman.velocity =
        Vec3(
            actualVelocity.x,
            actualVelocity.y,
            actualVelocity.z
        );

    // ------------------------------------------------------------
    // 7. CINEMATIC BODY ORIENTATION
    // ------------------------------------------------------------
    // Follow the real velocity when moving. During a stationary hover use
    // camera heading instead of trying to LookRotation a zero vector.
    float targetLean =
        -sideInput * g_superman.leanAmount;

    g_flightLean +=
        (targetLean - g_flightLean) *
        MinF(1.0f, g_superman.leanSpeed * frame);

    Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);

    float targetPitch = camRot.x;

    if (actualSpeed > 5.0f)
    {
        float horizontalSpeed =
            std::sqrt(
                actualVelocity.x * actualVelocity.x +
                actualVelocity.y * actualVelocity.y
            );

        if (horizontalSpeed > 0.1f)
        {
            targetPitch =
                -std::atan2(
                    actualVelocity.z,
                    horizontalSpeed
                ) * 57.2957795f;
        }
    }

    g_flightPitch +=
        (targetPitch - g_flightPitch) *
        MinF(1.0f, g_superman.turnSpeed * frame);

    ENTITY::SET_ENTITY_ROTATION(
        ped,
        g_flightPitch,
        g_flightLean,
        camRot.z,
        2,
        true
    );

    // Boost effects remain tied to actual physical speed.
    BoostEffect(ped, boosting, actualSpeed);
}

static void UpdateAbilities(Ped ped, float dt)
{
    if (!g_superman.enabled)
        return;

    if (Pressed(g_keys.flight, lastFlight))
    {
        g_superman.abilities.state.flight =
            !g_superman.abilities.state.flight;
    }

    if (Pressed(g_keys.superSpeed, lastSuperSpeed))
    {
        g_superman.abilities.state.superSpeed =
            !g_superman.abilities.state.superSpeed;
    }

    if (Pressed(g_keys.invincible, lastInvincible))
    {
        g_superman.abilities.state.godMode =
            !g_superman.abilities.state.godMode;
    }

    if (Pressed(g_keys.superJump, lastSuperJump) &&
        !g_superman.abilities.state.flight)
    {
        GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(
            PLAYER::PLAYER_ID()
        );
    }

    if (Pressed(g_keys.groundPound, lastGroundPound))
    {
        if (g_superman.abilities.state.flight)
            GroundPound(ped);
    }

    if (Pressed(g_keys.sonicBoom, lastSonicBoom))
    {
        SonicBoom(ped);
    }

    if (Pressed(g_keys.solarFlare, lastSolarFlare))
    {
        SolarFlare(ped);
    }

    if (Pressed(g_keys.grab, lastGrab))
    {
        if (g_grabbedPed == 0)
            Grab(ped);
        else
            ThrowGrabbed(ped);
    }

    if (Pressed(g_keys.throwTarget, lastThrow))
    {
        ThrowGrabbed(ped);
    }

    if (Pressed(g_keys.bulletTime, lastBulletTime))
    {
        g_superman.abilities.state.bulletTime =
            !g_superman.abilities.state.bulletTime;
    }

    if (Pressed(g_keys.xrayVision, lastXRay))
    {
        g_superman.abilities.state.xrayVision =
            !g_superman.abilities.state.xrayVision;
    }

    // Beam/breath abilities are deliberately hold-to-use:
    // press and hold = active, release = immediately stops.
    g_superman.abilities.state.heatVision =
        Down(g_keys.heatVision);

    g_superman.abilities.state.freezeBreath =
        Down(g_keys.freezeBreath);

    g_superman.abilities.state.superBreath =
        Down(g_keys.superBreath);

    if (g_superman.abilities.state.godMode)
        ENTITY::SET_ENTITY_INVINCIBLE(ped, true);

    PED::SET_PED_MOVE_RATE_OVERRIDE(
        ped,
        g_superman.abilities.state.superSpeed
            ? 2.0f
            : 1.0f
    );

    UpdateFlight(ped, dt);

    if (g_superman.abilities.state.heatVision)
        HeatVision(ped);

    if (g_superman.abilities.state.freezeBreath)
        FreezeBreath(ped);
    else if (g_frozenPed != 0 &&
             g_lastGameTime >= g_freezeReleaseTime)
        ReleaseFrozenTarget();

    if (g_superman.abilities.state.superBreath)
        SuperBreath(ped);

    if (g_frozenPed != 0 &&
        g_lastGameTime >= g_freezeReleaseTime)
        ReleaseFrozenTarget();

    UpdateXRay(ped, g_superman.abilities.state.xrayVision);

    if (g_superman.abilities.state.bulletTime)
        GAMEPLAY::SET_TIME_SCALE(0.35f);
    else
        GAMEPLAY::SET_TIME_SCALE(1.0f);

    // Ground-pound impact when the character reaches the ground.
    static bool wasPounding = false;

    if (g_superman.abilities.state.flight &&
        g_superman.velocity.z < -20.0f)
    {
        wasPounding = true;
    }

    if (wasPounding &&
        ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(ped) < 1.2f)
    {
        GroundPoundImpact(ped);
        wasPounding = false;
    }

    (void)dt;
}

static void MainUpdate()
{
    Ped ped = PLAYER::PLAYER_PED_ID();

    if (!ENTITY::DOES_ENTITY_EXIST(ped))
        return;

    DWORD now =
        (DWORD)GAMEPLAY::GET_GAME_TIMER();

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

    // Direct controls can start Superman without opening the menu.
    // Hold-to-use abilities also wake the system immediately.
    if (!g_superman.enabled)
    {
        bool directAbilityInput =
            Down(g_keys.flight) ||
            Down(g_keys.superSpeed) ||
            Down(g_keys.invincible) ||
            Down(g_keys.superJump) ||
            Down(g_keys.heatVision) ||
            Down(g_keys.freezeBreath) ||
            Down(g_keys.superBreath) ||
            Down(g_keys.groundPound) ||
            Down(g_keys.grab) ||
            Down(g_keys.throwTarget) ||
            Down(g_keys.sonicBoom) ||
            Down(g_keys.solarFlare) ||
            Down(g_keys.bulletTime) ||
            Down(g_keys.xrayVision);

        if (directAbilityInput)
            SupermanOn(ped);
    }

    if (g_superman.enabled)
        UpdateAbilities(ped, dt);

    DrawMenu();
}

void ScriptMain()
{
    LoadControls();

    g_superman.Reset();

    g_lastGameTime =
        (DWORD)GAMEPLAY::GET_GAME_TIMER();

    // Initialize edge-detection states from the current keyboard state.
    // This prevents a key held while GTA finishes loading from immediately
    // toggling the menu or an ability.
    lastMenu = Down(g_keys.menu);
    lastF3 = Down(VK_F3);
    lastF5 = Down(VK_F5);
    lastBack = Down(VK_BACK);
    lastUp = Down(VK_UP);
    lastDown = Down(VK_DOWN);
    lastEnter = Down(VK_RETURN);
    lastFlight = Down(g_keys.flight);
    lastSuperSpeed = Down(g_keys.superSpeed);
    lastInvincible = Down(g_keys.invincible);
    lastSuperJump = Down(g_keys.superJump);
    lastGroundPound = Down(g_keys.groundPound);
    lastSonicBoom = Down(g_keys.sonicBoom);
    lastSolarFlare = Down(g_keys.solarFlare);
    lastGrab = Down(g_keys.grab);
    lastThrow = Down(g_keys.throwTarget);
    lastBulletTime = Down(g_keys.bulletTime);
    lastXRay = Down(g_keys.xrayVision);

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
