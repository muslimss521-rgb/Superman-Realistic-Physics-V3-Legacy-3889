#include "FlightAnimation.h"
#include "natives.h"

namespace FlightAnimation
{
    // Superman-like built-in GTA V stunt pose.
    static char kSuperDict[] = "veh@bike@tricks";
    static char kSuperAnim[] = "tricks_superman";

    // Safe fallback for game builds where the stunt animation cannot be loaded.
    static char kFallbackDict[] = "skydive@freefall";
    static char kFallbackAnim[] = "free_forward";

    static bool g_superLoaded = false;
    static bool g_fallbackLoaded = false;
    static bool g_playing = false;
    static bool g_usingFallback = false;

    static void RequestAnims()
    {
        if (!g_superLoaded)
        {
            STREAMING::REQUEST_ANIM_DICT(kSuperDict);

            if (STREAMING::HAS_ANIM_DICT_LOADED(kSuperDict))
                g_superLoaded = true;
        }

        if (!g_fallbackLoaded)
        {
            STREAMING::REQUEST_ANIM_DICT(kFallbackDict);

            if (STREAMING::HAS_ANIM_DICT_LOADED(kFallbackDict))
                g_fallbackLoaded = true;
        }
    }

    static char* ActiveDict()
    {
        return g_usingFallback ? kFallbackDict : kSuperDict;
    }

    static char* ActiveAnim()
    {
        return g_usingFallback ? kFallbackAnim : kSuperAnim;
    }

    static bool SelectAnimation()
    {
        if (g_superLoaded)
        {
            g_usingFallback = false;
            return true;
        }

        if (g_fallbackLoaded)
        {
            g_usingFallback = true;
            return true;
        }

        return false;
    }

    static void StopCurrent(Ped ped)
    {
        if (!g_playing)
            return;

        AI::STOP_ANIM_TASK(
            ped,
            ActiveDict(),
            ActiveAnim(),
            1.0f
        );

        g_playing = false;
    }

    static void PlayFlight(Ped ped, bool boosting)
    {
        if (!SelectAnimation())
            return;

        if (!g_playing)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                ActiveDict(),
                ActiveAnim(),
                8.0f,
                -8.0f,
                -1,
                1 | 2 | 16 | 32,
                0.0f,
                FALSE,
                FALSE,
                FALSE
            );

            g_playing = true;
        }

        ENTITY::SET_ENTITY_ANIM_SPEED(
            ped,
            ActiveDict(),
            ActiveAnim(),
            boosting ? 1.35f : 0.90f
        );
    }

    void Initialize()
    {
        g_superLoaded = false;
        g_fallbackLoaded = false;
        g_playing = false;
        g_usingFallback = false;

        RequestAnims();
    }

    void Update(bool flying, bool boosting)
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        if (!flying)
        {
            StopCurrent(ped);
            PED::SET_PED_CAN_RAGDOLL(ped, TRUE);

            Vector3 rot = ENTITY::GET_ENTITY_ROTATION(ped, 2);

            ENTITY::SET_ENTITY_ROTATION(
                ped,
                0.0f,
                0.0f,
                rot.z,
                2,
                TRUE
            );

            return;
        }

        RequestAnims();

        if (!SelectAnimation())
            return;

        PED::SET_PED_CAN_RAGDOLL(ped, FALSE);
        PlayFlight(ped, boosting);
    }

    void Stop()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        StopCurrent(ped);
        PED::SET_PED_CAN_RAGDOLL(ped, TRUE);

        Vector3 rot = ENTITY::GET_ENTITY_ROTATION(ped, 2);

        ENTITY::SET_ENTITY_ROTATION(
            ped,
            0.0f,
            0.0f,
            rot.z,
            2,
            TRUE
        );
    }

    void Shutdown()
    {
        Stop();

        if (g_superLoaded)
            STREAMING::REMOVE_ANIM_DICT(kSuperDict);

        if (g_fallbackLoaded)
            STREAMING::REMOVE_ANIM_DICT(kFallbackDict);

        g_superLoaded = false;
        g_fallbackLoaded = false;
        g_playing = false;
        g_usingFallback = false;
    }
}
