#include "FlightAnimation.h"
#include "natives.h"
#include "../Input.h"

namespace FlightAnimation
{
    // Native GTA V animation that has a Superman-like horizontal pose.
    // Fallback is the normal freefall animation if the stunt dictionary
    // is unavailable in a particular game build.
    static char kSuperDict[] = "veh@bike@tricks";
    static char kSuperAnim[] = "tricks_superman";

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

    static const char* ActiveDict()
    {
        return g_usingFallback ? kFallbackDict : kSuperDict;
    }

    static const char* ActiveAnim()
    {
        return g_usingFallback ? kFallbackAnim : kSuperAnim;
    }

    static bool CanPlay()
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
            const_cast<char*>(ActiveDict()),
            const_cast<char*>(ActiveAnim()),
            1.5f
        );

        g_playing = false;
    }

    static void PlaySuperman(Ped ped, bool boosting)
    {
        if (!CanPlay())
            return;

        if (!g_playing)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                const_cast<char*>(ActiveDict()),
                const_cast<char*>(ActiveAnim()),
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
            const_cast<char*>(ActiveDict()),
            const_cast<char*>(ActiveAnim()),
            boosting ? 1.35f : 0.85f
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

            // Restore normal upright orientation after flight.
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

        if (!CanPlay())
            return;

        PED::SET_PED_CAN_RAGDOLL(ped, FALSE);
        PlaySuperman(ped, boosting);
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
