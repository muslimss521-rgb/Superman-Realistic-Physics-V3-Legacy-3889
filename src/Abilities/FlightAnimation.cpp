#include "FlightAnimation.h"
#include "../Input.h"
#include "main.h"
#include "natives.h"

namespace FlightAnimation
{
    // GTA V built-in freefall/parachute animation set.
    // No external animation files are required.
    static const char* kDict = "skydive@parachute@freefall";

    static const char* kIdle     = "free_idle";
    static const char* kForward  = "free_forward";
    static const char* kBackward = "free_backward";
    static const char* kLeft     = "free_left";
    static const char* kRight    = "free_right";

    static bool g_loaded = false;
    static const char* g_current = nullptr;

    static char* Mutable(const char* text)
    {
        return const_cast<char*>(text);
    }

    static void Request()
    {
        if (g_loaded)
            return;

        char* dict = Mutable(kDict);
        STREAMING::REQUEST_ANIM_DICT(dict);

        if (STREAMING::HAS_ANIM_DICT_LOADED(dict))
            g_loaded = true;
    }

    static void Play(Ped ped, const char* clip, float rate)
    {
        if (!g_loaded || clip == nullptr)
            return;

        char* dict = Mutable(kDict);
        char* anim = Mutable(clip);

        if (g_current == clip &&
            ENTITY::IS_ENTITY_PLAYING_ANIM(ped, dict, anim, 3))
        {
            ENTITY::SET_ENTITY_ANIM_SPEED(ped, dict, anim, rate);
            return;
        }

        // AI namespace is where this SDK exposes TASK_* natives.
        AI::TASK_PLAY_ANIM(
            ped,
            dict,
            anim,
            8.0f,
            -8.0f,
            -1,
            1 | 2 | 16 | 32,
            0.0f,
            FALSE,
            FALSE,
            FALSE);

        g_current = clip;
    }

    void Initialize()
    {
        g_current = nullptr;
        g_loaded = false;
        Request();
    }

    void Shutdown()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (ENTITY::DOES_ENTITY_EXIST(ped) && g_current != nullptr)
        {
            AI::STOP_ANIM_TASK(
                ped,
                Mutable(kDict),
                Mutable(g_current),
                2.0f);
        }

        g_current = nullptr;

        if (g_loaded)
            STREAMING::REMOVE_ANIM_DICT(Mutable(kDict));

        g_loaded = false;
    }

    void Update(
        bool flying,
        bool boosting,
        bool forward,
        bool back,
        bool left,
        bool right,
        bool up,
        bool down)
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (!ENTITY::DOES_ENTITY_EXIST(ped))
            return;

        if (!flying)
        {
            if (g_current != nullptr)
            {
                AI::STOP_ANIM_TASK(
                    ped,
                    Mutable(kDict),
                    Mutable(g_current),
                    2.0f);

                g_current = nullptr;
            }

            PED::SET_PED_CAN_RAGDOLL(ped, TRUE);
            return;
        }

        Request();

        if (!g_loaded)
            return;

        // Keep the player stable while the flight animation is active.
        PED::SET_PED_CAN_RAGDOLL(ped, FALSE);

        const char* clip = kIdle;

        if (forward)
            clip = kForward;
        else if (back)
            clip = kBackward;
        else if (left)
            clip = kLeft;
        else if (right)
            clip = kRight;
        else if (up || down)
            clip = kIdle;

        // Boost increases animation playback speed.
        const float rate = boosting ? 1.35f : 1.0f;

        Play(ped, clip, rate);
    }
}
