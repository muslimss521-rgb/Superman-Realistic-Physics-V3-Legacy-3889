#pragma once

namespace Flight
{
    void Initialize();
    void Enable();
    void Disable();
    void Update(bool boost);

    bool IsEnabled();
}
