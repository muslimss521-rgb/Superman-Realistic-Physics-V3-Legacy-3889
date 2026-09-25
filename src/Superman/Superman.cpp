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
    // Enabled at game start so the player can test the abilities immediately.
    static bool g_enabled = true;
    static bool g_speed = false;

    void Initialize()
    {
        g_enabled = true;
        g_speed = false;
        FlightAnimation::Initialize();
    }

    void Update()
    {
        if (Input::ToggleSuperman())
            g_enabled = !g_enabled;

        if (Input::ToggleFlight())
        {
            if (Flight::IsEnabled())
                Flight::Disable();
            else if (g_enabled)
                Flight::Enable();
        }

        if (Input::ToggleTargetLock())
            TargetLock::Toggle();

        if (Input::ToggleHeatVision())
            HeatVision::Toggle();

        if (Input::ToggleSuperSpeed())
            g_speed = !g_speed;

        if (Input::SuperPunch())
            Combat::SuperPunch();

        if (Input::EmergencyOff())
        {
            g_enabled = false;
            g_speed = false;
            Flight::Disable();
            HeatVision::Disable();
            TargetLock::Disable();
        }

        if (!g_enabled)
        {
            Flight::Disable();

            FlightAnimation::Update(
                false, false,
                false, false, false, false, false, false);

            TargetLock::Update();
            HeatVision::Update();

            PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(
                PLAYER::PLAYER_ID(), 1.0f);

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

        PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(
            PLAYER::PLAYER_ID(),
            g_speed ? 1.49f : 1.0f);
    }
}
