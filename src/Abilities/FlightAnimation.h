#pragma once

namespace FlightAnimation
{
    void Initialize();
    void Shutdown();
    void Update(bool flying, bool boosting, bool forward, bool back, bool left, bool right, bool up, bool down);
}
