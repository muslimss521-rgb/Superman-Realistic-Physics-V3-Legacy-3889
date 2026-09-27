#include "FlightAnimation.h"
#include "../Input.h"
#include "../../include/ScriptHookV/natives.h"

namespace FlightAnimation
{
    static bool g_playing = false;
    static const char* kDict = "skydive@freefall";
    static const char* kAnim = "free_forward";

    void Initialize()
    {
        g_playing = false;
    }

    static void EnsureLoaded()
    {
        if (!STREAMING::HAS_ANIM_DICT_LOADED(kDict))
            STREAMING::REQUEST_ANIM_DICT(kDict);
    }

    void Update(bool flying, bool boosting)
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!flying)
        {
            if (g_playing)
                AI::STOP_ANIM_TASK(ped, kDict, kAnim, -4.0f);
            g_playing = false;
            PED::SET_PED_CAN_RAGDOLL(ped, true);
            return;
        }

        EnsureLoaded();

        if (!STREAMING::HAS_ANIM_DICT_LOADED(kDict))
            return;

        PED::SET_PED_CAN_RAGDOLL(ped, false);

        // Reapply only when needed so the animation does not restart every frame.
        if (!g_playing)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                kDict,
                kAnim,
                8.0f,
                -8.0f,
                -1,
                1 | 32 | 64,
                1.0f,
                false,
                false,
                false
            );
            g_playing = true;
        }

        // Boost gives a slightly stronger forward-flight animation speed.
        AI::SET_ANIM_RATE(ped, kDict, kAnim, boosting ? 1.35f : 1.0f);
    }

    void Stop()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();
        if (g_playing)
            AI::STOP_ANIM_TASK(ped, kDict, kAnim, -4.0f);

        g_playing = false;
        PED::SET_PED_CAN_RAGDOLL(ped, true);
    }

    void Shutdown()
    {
        Stop();
    }
}
