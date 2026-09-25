#include "Superman.h"
#include "Input.h"
#include "../Abilities/Flight.h"

namespace
{
    bool g_enabled = false;
}

namespace Superman
{
    void Initialize()
    {
        Flight::Initialize();
    }

    void Update()
    {
        Input::Update();

        if (Input::Pressed(Input::Key::Toggle))
        {
            g_enabled = !g_enabled;

            if (!g_enabled)
                Flight::Disable();
        }

        if (!g_enabled)
            return;

        if (Input::Pressed(Input::Key::Flight))
        {
            if (Flight::IsEnabled())
                Flight::Disable();
            else
                Flight::Enable();
        }

        Flight::Update(Input::Down(Input::Key::Boost));
    }

    bool Enabled() { return g_enabled; }
    bool FlightEnabled() { return Flight::IsEnabled(); }
}
