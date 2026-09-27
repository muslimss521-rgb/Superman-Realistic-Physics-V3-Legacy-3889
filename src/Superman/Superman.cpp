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
    // The mod starts OFF. F3 toggles the master switch.
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
        // F3 = master Superman ON/OFF
        if (Input::ToggleSuperman())
            g_enabled = !g_enabled;

        // F = flight ON/OFF
        if (Input::ToggleFlight())
        {
            if (Flight::IsEnabled())
                Flight::Disable();
            else if (g_enabled)
                Flight::Enable();
        }

        // T = target lock
        if (Input::ToggleTargetLock())
            TargetLock::Toggle();

        // H = heat vision
        if (Input::ToggleHeatVision())
            HeatVision::Toggle();

        // C = super speed
        if (Input::ToggleSuperSpeed())
            g_speed = !g_speed;

        // R = super punch
        if (Input::SuperPunch())
            Combat::SuperPunch();

        // X = emergency OFF
        if (Input::EmergencyOff())
        {
            g_enabled = false;
            g_speed = false;
            Flight::Disable();
            FlightAnimation::Stop();
            HeatVision::Disable();
            TargetLock::Disable();
        }

        // Master switch OFF: keep all Superman abilities disabled.
        if (!g_enabled)
        {
            Flight::Disable();
            FlightAnimation::Update(false, false);
            TargetLock::Disable();
            HeatVision::Disable();

            PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(
                PLAYER::PLAYER_ID(), 1.0f);

            return;
        }

        const bool flying = Flight::IsEnabled();
        const bool boosting = Input::Boost();

        Flight::SetBoost(boosting);
        Flight::Update();

        // FlightAnimation::Update now takes only:
        // (flying, boosting)
        FlightAnimation::Update(flying, boosting);

        TargetLock::Update();
        HeatVision::Update();

        PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(
            PLAYER::PLAYER_ID(),
            g_speed ? 1.49f : 1.0f);
    }
}
