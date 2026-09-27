#include "FlightAnimation.h"
#include "natives.h"

namespace FlightAnimation
{
    // The real JulioNIB V2.1 video uses custom flight animation dictionaries.
    // We do NOT require NIBSHDotNet here. If those custom dictionaries are not
    // installed, the code falls back to GTA V built-in animations.

    static char kCustomIdleDict[]  = "export@nib@super@basicflight_idle";
    static char kCustomIdleClip[]  = "basicflight_idle";

    static char kCustomFlightDict[] = "export@nib@super@basicflight";
    static char kCustomFlightClip[] = "basicflight";

    // Built-in fallbacks:
    // Hover: arms-up standing pose while gravity is disabled.
    static char kHoverDict[] = "amb@world_human_cheering@male_a";
    static char kHoverClip[] = "base";

    // Flight: actual airborne pose.
    static char kFlightDict[] = "skydive@freefall";
    static char kFlightClip[] = "free_forward";

    static bool g_hoverLoaded = false;
    static bool g_flightLoaded = false;
    static bool g_customIdleLoaded = false;
    static bool g_customFlightLoaded = false;

    enum AnimState
    {
        STATE_NONE = 0,
        STATE_HOVER,
        STATE_FLIGHT
    };

    static AnimState g_state = STATE_NONE;

    static void Request(char* dict, bool& loaded)
    {
        if (loaded)
            return;

        STREAMING::REQUEST_ANIM_DICT(dict);

        if (STREAMING::HAS_ANIM_DICT_LOADED(dict))
            loaded = true;
    }

    static void StopAnimation(Ped ped)
    {
        if (g_state == STATE_NONE)
            return;

        if (g_state == STATE_HOVER)
        {
            if (g_customIdleLoaded)
                AI::STOP_ANIM_TASK(ped, kCustomIdleDict, kCustomIdleClip, 1.5f);
            if (g_hoverLoaded)
                AI::STOP_ANIM_TASK(ped, kHoverDict, kHoverClip, 1.5f);
        }
        else if (g_state == STATE_FLIGHT)
        {
            if (g_customFlightLoaded)
                AI::STOP_ANIM_TASK(ped, kCustomFlightDict, kCustomFlightClip, 1.5f);
            if (g_flightLoaded)
                AI::STOP_ANIM_TASK(ped, kFlightDict, kFlightClip, 1.5f);
        }

        g_state = STATE_NONE;
    }

    static void PlayHover(Ped ped)
    {
        if (g_state == STATE_HOVER)
            return;

        StopAnimation(ped);

        // Prefer the custom Superman hover if it happens to be installed.
        if (g_customIdleLoaded)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                kCustomIdleDict,
                kCustomIdleClip,
                6.0f,
                -6.0f,
                -1,
                1 | 2 | 4 | 16,
                1.0f,
                FALSE,
                FALSE,
                FALSE
            );

            g_state = STATE_HOVER;
            return;
        }

        // Built-in approximation of the video:
        // upright floating body + arms raised, with gravity disabled.
        if (g_hoverLoaded)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                kHoverDict,
                kHoverClip,
                6.0f,
                -6.0f,
                -1,
                1 | 2 | 4 | 16,
                1.0f,
                FALSE,
                FALSE,
                FALSE
            );

            g_state = STATE_HOVER;
        }
    }

    static void PlayFlight(Ped ped, bool boosting)
    {
        if (g_state == STATE_FLIGHT)
        {
            ENTITY::SET_ENTITY_ANIM_SPEED(
                ped,
                g_customFlightLoaded ? kCustomFlightDict : kFlightDict,
                g_customFlightLoaded ? kCustomFlightClip : kFlightClip,
                boosting ? 1.55f : 1.15f
            );
            return;
        }

        StopAnimation(ped);

        // Prefer the custom Superman fast-flight animation if installed.
        if (g_customFlightLoaded)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                kCustomFlightDict,
                kCustomFlightClip,
                5.0f,
                -5.0f,
                -1,
                1 | 2 | 4 | 16,
                1.0f,
                FALSE,
                FALSE,
                FALSE
            );

            g_state = STATE_FLIGHT;

            ENTITY::SET_ENTITY_ANIM_SPEED(
                ped,
                kCustomFlightDict,
                kCustomFlightClip,
                boosting ? 1.55f : 1.15f
            );
            return;
        }

        if (g_flightLoaded)
        {
            AI::TASK_PLAY_ANIM(
                ped,
                kFlightDict,
                kFlightClip,
                5.0f,
                -5.0f,
                -1,
                1 | 2 | 4 | 16,
                1.0f,
                FALSE,
                FALSE,
                FALSE
            );

            g_state = STATE_FLIGHT;

            ENTITY::SET_ENTITY_ANIM_SPEED(
                ped,
                kFlightDict,
                kFlightClip,
                boosting ? 1.55f : 1.15f
            );
        }
    }

    void Initialize()
    {
        g_hoverLoaded = false;
        g_flightLoaded = false;
        g_customIdleLoaded = false;
        g_customFlightLoaded = false;
        g_state = STATE_NONE;

        // Try both custom dictionaries first. Failure is harmless.
        Request(kCustomIdleDict, g_customIdleLoaded);
        Request(kCustomFlightDict, g_customFlightLoaded);

        // Always prepare built-in fallbacks.
        Request(kHoverDict, g_hoverLoaded);
        Request(kFlightDict, g_flightLoaded);
    }

    void Update(bool flying, bool boosting)
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        if (!flying)
        {
            StopAnimation(ped);
            return;
        }

        // Try loading dictionaries again because the first request can finish
        // after Initialize().
        Request(kCustomIdleDict, g_customIdleLoaded);
        Request(kCustomFlightDict, g_customFlightLoaded);
        Request(kHoverDict, g_hoverLoaded);
        Request(kFlightDict, g_flightLoaded);

        // Determine state from actual movement.
        // Boost forces the fast-flight pose immediately.
        const float speed = ENTITY::GET_ENTITY_SPEED(ped);
        const bool fastFlight = boosting || speed > 4.0f;

        if (fastFlight)
            PlayFlight(ped, boosting);
        else
            PlayHover(ped);
    }

    void Stop()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (ENTITY::DOES_ENTITY_EXIST(ped))
            StopAnimation(ped);
    }

    void Shutdown()
    {
        Stop();

        if (g_customIdleLoaded)
            STREAMING::REMOVE_ANIM_DICT(kCustomIdleDict);

        if (g_customFlightLoaded)
            STREAMING::REMOVE_ANIM_DICT(kCustomFlightDict);

        if (g_hoverLoaded)
            STREAMING::REMOVE_ANIM_DICT(kHoverDict);

        if (g_flightLoaded)
            STREAMING::REMOVE_ANIM_DICT(kFlightDict);

        g_customIdleLoaded = false;
        g_customFlightLoaded = false;
        g_hoverLoaded = false;
        g_flightLoaded = false;
        g_state = STATE_NONE;
    }
}
