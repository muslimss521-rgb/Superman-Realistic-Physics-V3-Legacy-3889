#include "FlightAnimation.h"
#include "natives.h"
#include "../Input.h"

namespace FlightAnimation
{
    // Built-in GTA V stunt pose that is explicitly named "Superman".
    static char kDict[] = "veh@bike@tricks";
    static char kAnim[] = "tricks_superman";

    // Reliable fallback only if the Superman stunt clip is unavailable.
    static char kFallbackDict[] = "skydive@freefall";
    static char kFallbackAnim[] = "free_forward";

    static bool g_superLoaded = false;
    static bool g_fallbackLoaded = false;
    static bool g_playing = false;
    static bool g_fallback = false;

    static void Request()
    {
        if (!g_superLoaded)
        {
            STREAMING::REQUEST_ANIM_DICT(kDict);
            if (STREAMING::HAS_ANIM_DICT_LOADED(kDict))
                g_superLoaded = true;
        }

        if (!g_fallbackLoaded)
        {
            STREAMING::REQUEST_ANIM_DICT(kFallbackDict);
            if (STREAMING::HAS_ANIM_DICT_LOADED(kFallbackDict))
                g_fallbackLoaded = true;
        }
    }

    static char* Dict()
    {
        return g_fallback ? kFallbackDict : kDict;
    }

    static char* Anim()
    {
        return g_fallback ? kFallbackAnim : kAnim;
    }

    static bool SelectAnimation()
    {
        if (g_superLoaded)
        {
            g_fallback = false;
            return true;
        }

        if (g_fallbackLoaded)
        {
            g_fallback = true;
            return true;
        }

        return false;
    }

    static void StopCurrent(Ped ped)
    {
        if (!g_playing)
            return;

        AI::STOP_ANIM_TASK(ped, Dict(), Anim(), 0.8f);
        g_playing = false;
    }

    static void Start(Ped ped, bool boosting)
    {
        if (!SelectAnimation())
            return;

        if (!g_playing)
        {
            // Loop + hold pose. Do not use the upper-body-only flag:
            // the Superman stunt is a full-body flight pose.
            AI::TASK_PLAY_ANIM(
                ped,
                Dict(),
                Anim(),
                8.0f,
                -8.0f,
                -1,
                1 | 2 | 8,
                0.0f,
                FALSE,
                FALSE,
                FALSE
            );

            g_playing = true;
        }

        // Slow, controlled pose in normal flight; faster cycle while boosting.
        ENTITY::SET_ENTITY_ANIM_SPEED(
            ped,
            Dict(),
            Anim(),
            boosting ? 1.55f : 0.95f
        );
    }

    void Initialize()
    {
        g_superLoaded = false;
        g_fallbackLoaded = false;
        g_playing = false;
        g_fallback = false;
        Request();
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
            ENTITY::SET_ENTITY_ROTATION(ped, 0.0f, 0.0f, rot.z, 2, TRUE);
            return;
        }

        Request();

        if (!SelectAnimation())
            return;

        PED::SET_PED_CAN_RAGDOLL(ped, FALSE);
        Start(ped, boosting);
    }

    void Stop()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();
        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        StopCurrent(ped);
        PED::SET_PED_CAN_RAGDOLL(ped, TRUE);

        Vector3 rot = ENTITY::GET_ENTITY_ROTATION(ped, 2);
        ENTITY::SET_ENTITY_ROTATION(ped, 0.0f, 0.0f, rot.z, 2, TRUE);
    }

    void Shutdown()
    {
        Stop();

        if (g_superLoaded)
            STREAMING::REMOVE_ANIM_DICT(kDict);

        if (g_fallbackLoaded)
            STREAMING::REMOVE_ANIM_DICT(kFallbackDict);

        g_superLoaded = false;
        g_fallbackLoaded = false;
        g_playing = false;
        g_fallback = false;
    }
}
