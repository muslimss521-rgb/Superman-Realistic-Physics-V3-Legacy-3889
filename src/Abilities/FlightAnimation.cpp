#include "FlightAnimation.h"
#include "main.h"
#include "natives.h"

namespace FlightAnimation
{
    // Verified GTA V freefall dictionary.
    static const char* kDict = "skydive@freefall";
    static const char* kAnim = "free_forward";

    static bool g_loaded = false;
    static bool g_playing = false;

    static char* M(const char* s)
    {
        return const_cast<char*>(s);
    }

    static void Request()
    {
        if (g_loaded)
            return;

        STREAMING::REQUEST_ANIM_DICT(M(kDict));
        if (STREAMING::HAS_ANIM_DICT_LOADED(M(kDict)))
            g_loaded = true;
    }

    void Initialize()
    {
        g_loaded = false;
        g_playing = false;
        Request();
    }

    void Shutdown()
    {
        Ped ped = PLAYER::PLAYER_PED_ID();

        if (ENTITY::DOES_ENTITY_EXIST(ped) && g_playing)
        {
            AI::STOP_ANIM_TASK(ped, M(kDict), M(kAnim), 2.0f);
        }

        g_playing = false;

        if (g_loaded)
            STREAMING::REMOVE_ANIM_DICT(M(kDict));

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
            if (g_playing)
            {
                AI::STOP_ANIM_TASK(ped, M(kDict), M(kAnim), 2.0f);
                g_playing = false;
            }

            PED::SET_PED_CAN_RAGDOLL(ped, TRUE);
            return;
        }

        Request();
        if (!g_loaded)
            return;

        PED::SET_PED_CAN_RAGDOLL(ped, FALSE);

        if (!g_playing ||
            !ENTITY::IS_ENTITY_PLAYING_ANIM(ped, M(kDict), M(kAnim), 3))
        {
            // Loop + controllable full-body animation.
            const int flags = 1 | 32 | 64;
            AI::TASK_PLAY_ANIM(
                ped,
                M(kDict),
                M(kAnim),
                8.0f,
                -8.0f,
                -1,
                flags,
                boosting ? 1.20f : 1.0f,
                FALSE,
                FALSE,
                FALSE);

            g_playing = true;
        }
        else
        {
            ENTITY::SET_ENTITY_ANIM_SPEED(
                ped,
                M(kDict),
                M(kAnim),
                boosting ? 1.20f : 1.0f);
        }
    }
}
