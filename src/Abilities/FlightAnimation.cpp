#include "FlightAnimation.h"
#include "main.h"

namespace FlightAnimation
{
    // GTA V built-in flight/freefall animation.
    // Mutable char buffers are required by this ScriptHookV SDK.
    static char kDict[] = "skydive@freefall";
    static char kIdle[] = "free_forward";

    static bool g_loaded = false;
    static bool g_playing = false;

    static void RequestAnim()
    {
        if (!g_loaded)
        {
            STREAMING::REQUEST_ANIM_DICT(kDict);

            if (STREAMING::HAS_ANIM_DICT_LOADED(kDict))
                g_loaded = true;
        }
    }

    static void StopCurrent(Ped ped)
    {
        if (g_playing)
        {
            AI::STOP_ANIM_TASK(
                ped,
                kDict,
                kIdle,
                2.0f);

            g_playing = false;
        }
    }

    static void PlayFlight(Ped ped, bool boosting)
    {
        if (!g_loaded)
            return;

        if (!g_playing)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                kDict,
                kIdle,
                8.0f,
                -8.0f,
                -1,
                1 | 2 | 16 | 32,
                0.0f,
                FALSE,
                FALSE,
                FALSE);

            g_playing = true;
        }

        // Use the native that actually exists in this SDK.
        ENTITY::SET_ENTITY_ANIM_SPEED(
            ped,
            kDict,
            kIdle,
            boosting ? 1.35f : 1.0f);
    }

    void Initialize()
    {
        g_loaded = false;
        g_playing = false;
        RequestAnim();
    }

    void Update(bool flying, bool boosting)
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        if (!flying)
        {
            StopCurrent(ped);
            PED::SET_PED_CAN_RAGDOLL(ped, true);
            return;
        }

        RequestAnim();

        if (!g_loaded)
            return;

        PED::SET_PED_CAN_RAGDOLL(ped, false);
        PlayFlight(ped, boosting);
    }

    void Stop()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (ENTITY::DOES_ENTITY_EXIST(ped))
        {
            StopCurrent(ped);
            PED::SET_PED_CAN_RAGDOLL(ped, true);
        }
    }

    void Shutdown()
    {
        Stop();

        if (g_loaded)
            STREAMING::REMOVE_ANIM_DICT(kDict);

        g_loaded = false;
    }
}
