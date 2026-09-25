#include "Superman.h"
#include "../Input.h"
#include "../Abilities/Flight.h"
#include "../Abilities/TargetLock.h"
#include "../Abilities/HeatVision.h"
#include "../Abilities/Combat.h"
#include "../Abilities/FlightAnimation.h"

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
        // G - Superman master ON/OFF
        if (Input::ToggleSuperman())
            g_enabled = !g_enabled;

        // F - Flight ON/OFF
        if (Input::ToggleFlight())
        {
            if (Flight::IsEnabled())
                Flight::Disable();
            else if (g_enabled)
                Flight::Enable();
        }

        // T - Target Lock
        if (Input::ToggleTargetLock())
            TargetLock::Toggle();

        // H - Heat Vision
        if (Input::ToggleHeatVision())
            HeatVision::Toggle();

        // C - Super Speed
        if (Input::ToggleSuperSpeed())
            g_speed = !g_speed;

        // R - Super Punch
        if (Input::SuperPunch())
            Combat::SuperPunch();

        // X - Emergency OFF
        if (Input::EmergencyOff())
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
            // Speed state is retained here.
            // The existing speed implementation can be connected
            // without changing keyboard controls.
        }
    }
}
