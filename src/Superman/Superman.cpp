#include "Superman.h"
#include "../Input.h"
#include "../Abilities/Flight.h"
#include "../Abilities/TargetLock.h"
#include "../Abilities/HeatVision.h"
#include "../Abilities/Combat.h"
#include "../Abilities/FlightAnimation.h"
#include "main.h"
#include "natives.h"

namespace Superman
{
    static bool g_enabled = false;
    static bool g_speed = false;

    void Initialize()
    {
        g_enabled = false;
        g_speed = false;
        FlightAnimation::Initialize();
    }

    void Update()
    {
        // F3 - master toggle
        if (Input::JustPressed(170))
            g_enabled = !g_enabled;

        // F5 - flight
        if (Input::JustPressed(166))
        {
            if (Flight::IsEnabled())
                Flight::Disable();
            else if (g_enabled)
                Flight::Enable();
        }

        // F7 - target lock
        if (Input::JustPressed(168))
            TargetLock::Toggle();

        // F9 - heat vision
        if (Input::JustPressed(56))
            HeatVision::Toggle();

        // F10 - super speed
        if (Input::JustPressed(57))
            g_speed = !g_speed;

        // F6 - super punch
        if (Input::JustPressed(167))
            Combat::SuperPunch();

        // F8 - emergency off
        if (Input::JustPressed(169))
        {
            g_enabled = false;
            g_speed = false;
            Flight::Disable();
        }

        if (!g_enabled)
        {
            Flight::Disable();

            FlightAnimation::Update(
                false, false,
                false, false, false, false, false, false);

            TargetLock::Update();
            HeatVision::Update();
            return;
        }

        const bool flying = Flight::IsEnabled();
        const bool boosting = Input::Boost();

        Flight::SetBoost(boosting);
        Flight::Update();

        FlightAnimation::Update(
            flying,
            boosting,
            Input::Forward(),
            Input::Back(),
            Input::Left(),
            Input::Right(),
            Input::Up(),
            Input::Down());

        TargetLock::Update();
        HeatVision::Update();

        if (g_speed)
        {
            Ped ped = PLAYER::PLAYER_PED_ID();

            if (ENTITY::DOES_ENTITY_EXIST(ped))
            {
                // This native belongs to PLAYER in this ScriptHookV SDK.
                PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(
                    PLAYER::PLAYER_ID(),
                    1.49f);
            }
        }
    }
}
